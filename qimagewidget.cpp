#include "qimagewidget.h"

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//! RESIZABLE RUBBER BAND !//
QResizableRubberBand::QResizableRubberBand( QWidget *parent )
    : QWidget( parent )
{
    //tell QSizeGrip to resize this widget instead of top-level window
    setWindowFlags( Qt::SubWindow );
    setCursor( Qt::SizeAllCursor );
    setCursor( QCursor( QPixmap( ":/icons/cursor-move.png" ).scaled( 20, 20,
                                                                     Qt::KeepAspectRatio,
                                                                     Qt::SmoothTransformation ) ) );

    QHBoxLayout *layout = new QHBoxLayout( this );
    layout->setContentsMargins( 0, 0, 0, 0 );
    QSizeGrip *grip1 = new QSizeGrip( this );
    QSizeGrip *grip2 = new QSizeGrip( this );

    layout->addWidget( grip1, 0, Qt::AlignLeft | Qt::AlignTop );
    layout->addWidget( grip2, 0, Qt::AlignRight | Qt::AlignBottom );

    rubberBand = new QRubberBand( QRubberBand::Rectangle, this );
    rubberBand->setCursor( Qt::SizeAllCursor );
    rubberBand->move( 0, 0 );
    rubberBand->show();

    show();
}

//------------------------------------------------------------------------
void QResizableRubberBand::resizeEvent( QResizeEvent *event )
{
    rubberBand->resize( size() );
    event->accept();
}
//------------------------------------------------------------------------
void QResizableRubberBand::mousePressEvent( QMouseEvent *event )
{
    rubberBandOffset = event->pos();
    rubberBandMove = true;

    event->accept();
}
//------------------------------------------------------------------------
void QResizableRubberBand::mouseMoveEvent( QMouseEvent *event )
{
    if ( rubberBandMove ) {
        move( mapToParent( event->pos() - rubberBandOffset ) );
    }

    event->accept();
}
//------------------------------------------------------------------------
void QResizableRubberBand::mouseReleaseEvent( QMouseEvent *event )
{
    rubberBandMove = false;

    event->accept();
}

//------------------------------------------------------------------------
void QResizableRubberBand::mouseDoubleClickEvent( QMouseEvent *event )
{
    emit doubleClicked();

    event->accept();
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//! QCUSTOM GRAPHICS VIEW //!
QCustomGraphicsView::QCustomGraphicsView( QWidget *parent )
    : QGraphicsView( parent )
{
    //! [1]
    setRenderHints( QPainter::Antialiasing |
                    QPainter::SmoothPixmapTransform |
                    QPainter::TextAntialiasing );
    setDragMode( QGraphicsView::ScrollHandDrag );
    setTransformationAnchor( QGraphicsView::AnchorUnderMouse );
    setViewportUpdateMode( QGraphicsView::FullViewportUpdate );

    m_scene = new QGraphicsScene( this );
    setScene( m_scene );
    //! [1]

    //! [2]
    m_selector = new QResizableRubberBand( this );
    m_selector->close();
    m_selection = false;

    connect( m_selector, &QResizableRubberBand::doubleClicked,
             this, &QCustomGraphicsView::getRect );
    //! [2]

    //! [3]
    m_scaled = false;
    //! [3]
}

//---------------------------------------------------------------------------

QCustomGraphicsView::~QCustomGraphicsView()
{

}

//! [2]
void QCustomGraphicsView::startSelection()
{
    m_selection = true;
    viewport()->setCursor( Qt::CrossCursor );
}

void QCustomGraphicsView::stopSelection()
{
    m_selector->close();
    m_selection = false;

    viewport()->setCursor( QCursor() );
}

//---------------------------------------------------------------------------

void QCustomGraphicsView::mousePressEvent( QMouseEvent *event )
{
    if ( m_selection ) {
        m_selectionStartPosition = event->pos();
        m_selector->setGeometry( QRect( m_selectionStartPosition, QSize( 10, 10 ) ) );
        m_selector->show();
    } else {
        QGraphicsView::mousePressEvent( event );
    }

    event->accept();
}
//---------------------------------------------------------------------------

void QCustomGraphicsView::mouseMoveEvent( QMouseEvent *event )
{
    if ( m_selection ) {
        m_selector->setGeometry( QRect( m_selectionStartPosition, event->pos() ).normalized() );
    } else {
        QGraphicsView::mouseMoveEvent( event );
    }
}

//---------------------------------------------------------------------------

void QCustomGraphicsView::mouseReleaseEvent( QMouseEvent *event )
{
    m_selection = false;
    QGraphicsView::mouseReleaseEvent( event );
    event->accept();
}

void QCustomGraphicsView::keyReleaseEvent( QKeyEvent *event )
{
    if ( event->key() == Qt::Key_Escape ) {
        stopSelection();
    } else if ( event->key() == Qt::Key_Return ) {
        getRect();
    }

}

//---------------------------------------------------------------------------

void QCustomGraphicsView::getRect() // get selected rect
{
    QPolygonF p = mapToScene( m_selector->frameGeometry() );
    QRect r = QRect( p.at(0).toPoint(), p.at(2).toPoint() );

    stopSelection();

    emit selectedRect( r );
    emit selectedDone();
}

//---------------------------------------------------------------------------
//! [2]

//! [3]

void QCustomGraphicsView::zoomIn()
{
    scale( 1.15, 1.15 );
}

void QCustomGraphicsView::zoomOut()
{
    scale( 1 / 1.15, 1 / 1.15 );
}

void QCustomGraphicsView::wheelEvent( QWheelEvent *event )
{
    // todo: can resize even if selection
    if ( !m_selection ) {
        if ( event->delta() > 0 ) {
            zoomIn();
        } else {
            zoomOut();
        }
    }

    m_scaled = true;
    event->accept();
}

//! [3]
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//! QIMAGE WIDGET //!
QImageWidget::QImageWidget( QWidget *parent )
    : QWidget( parent )
{
    //! [1]
    // view and scene
    m_customGraphicsView = new QCustomGraphicsView( this );
    m_graphicsScene = new QGraphicsScene( this );
    m_customGraphicsView->setScene( m_graphicsScene );

    // preview widget
    m_previewWidget = new QListWidget( this );

    // layout
    m_splitter = new QSplitter( this );
    m_splitter->addWidget( m_customGraphicsView );
    m_splitter->addWidget( m_previewWidget );

    QHBoxLayout *m_layout = new QHBoxLayout( this );
    m_layout->addWidget( m_splitter );
    setLayout( m_layout );
    //! [1]

    //! [2]
    // filters
    m_filters = "*.png ; *.jpg ; *.bmp ; *.ico ; *.jpeg ; *.gif";
    m_subDirectorySearching = true;

    setCurrentPixmapModified( false );
    //! [2]

    //! [3]
    m_endlessScrollEnabled = false;
    //! [3]

    //! [4]
    m_scaleOnResize = true;
    m_scaled = false;
    //! [4]

    //! [5]
    connect( QApplication::clipboard(), &QClipboard::dataChanged,
             this, &QImageWidget::checkPasteAvailable );
    //! [5]

    //! [6]
    m_undoStack = new QUndoStack( this );
    //! [6]

    //! [7]

    //    m_rotateLeftCounter = 0;
    //    m_rotateRightCounter = 0;

    m_showRemoveDialog = true;
    m_moveToTrash = true;

    connect( m_customGraphicsView, &QCustomGraphicsView::selectedRect,
             this, &QImageWidget::getSelection );
    //! [7]

    //! [8]
    m_previewVisible = false;
    m_previewWidget->setVisible( false );
    m_pixmapsPathsBefore = QStringList();
    setPreviewPixmapSize( QSize( 100, 100 ) );

    connect( m_previewWidget, &QListWidget::currentRowChanged,
             this, &QImageWidget::currentPreviewChanged );

    //! [8]

    //! [9]
    m_infoBox = new QMessageBox( static_cast < QWidget * > ( this->parent() ) );
    //! [9]

    //! [11]
    m_previewThread = new QPreviewThread( this );
    connect( m_previewThread, &QPreviewThread::appendNewPreview,
             this, &QImageWidget::appendNewPreview );
    //! [11]

}
//---------------------------------------------------------------------------

QImageWidget::~QImageWidget()
{

}
//---------------------------------------------------------------------------

//! [2]
// setters
void QImageWidget::setPixmapsPaths( const QStringList &paths )
{
    if ( paths.isEmpty() ) {
        qWarning() << Q_FUNC_INFO << trUtf8( "No files." );
        return;
    }

    m_startedDirectoryPath = QFileInfo( paths.first() ).absolutePath();

    m_pixmapsPaths = paths;
    m_currentPixmapIndex = 0;

    updatePixmapByIndex();
}
//---------------------------------------------------------------------------

void QImageWidget::setPixmapsDirectory( const QString &dirPath )
{
    if ( dirPath.isEmpty() ) {
        qWarning() << Q_FUNC_INFO << trUtf8( "No directory or wrong path." );
        QMessageBox::critical( this,
                               trUtf8( "Error!"),
                               trUtf8( "No %1 directory or wrong path ").arg( dirPath ) );
        return;
    }

    m_pixmapsPaths.clear();
    m_pixmapsPaths = searchDirectory( dirPath );

    if ( m_pixmapsPaths.size() <= 0  ) {
        qWarning() << Q_FUNC_INFO << trUtf8( "No files!" );
        QMessageBox::critical( this,
                               trUtf8( "No images!"),
                               trUtf8( "Cannot find images at %1 directory ").arg( dirPath ) );
        return;
    }

    // for previews update
    m_startedDirectoryPath = dirPath;

    m_currentPixmapIndex = 0;
    if ( m_previewVisible && m_previewWidget->count() > 0 ) {
        m_previewWidget->currentRowChanged( m_currentPixmapIndex );
    } else {
        updatePixmapByIndex();
    }

    if ( m_pixmapsPaths.size() > 1 ) {
        emit goNextAvailable( true );
    } else if ( m_pixmapsPaths.size() == 1 ) {
        emit goNextAvailable( false );
        emit goPreviousAvailable( false );
    }
}

QStringList QImageWidget::searchDirectory( const QString &dirPath )
{
    QDirIterator::IteratorFlag searcherFlag;
    if ( m_subDirectorySearching ) {
        searcherFlag = QDirIterator::Subdirectories;
    } else {
        searcherFlag = QDirIterator::NoIteratorFlags;
    }

    QStringList paths;

    QDirIterator searcher( dirPath, m_filters.split( " ; " ), QDir::Files, searcherFlag  );
    while ( searcher.hasNext() ) {
        paths.append( searcher.next() );
    }

    return paths;
}

void QImageWidget::setPixmapPathWithDirectorySearching( const QString &path )
{
    if ( path.isEmpty() ) {
        qWarning() << Q_FUNC_INFO << trUtf8( "Empty path." );
        return;
    }

    // scan dir
    m_pixmapsPaths.clear();
    m_pixmapsPaths = searchDirectory( QFileInfo( path ).absolutePath() );
    if ( m_pixmapsPaths.size() <= 0  ) {
        qWarning() << Q_FUNC_INFO << trUtf8( "No files!" );
        QMessageBox::critical( this,
                               trUtf8( "Error!"),
                               trUtf8( "Look like %1 is no images and there is no images at %2" ).arg( path, QFileInfo( path ).absolutePath() ) );
        return;
    }

    // find current image and index
    for ( int i = 0; i < m_pixmapsPaths.size(); ++i ) {
        if ( path == m_pixmapsPaths.at( i ) ) {
            m_currentPixmapIndex = i;
            break;
        }
    }

    // for update previews
    m_startedDirectoryPath = QFileInfo( path ).absolutePath();

    // update pixmap by index
    updatePixmapByIndex();
}

void QImageWidget::setSubDirectorySearching( const bool &enable )
{
    m_subDirectorySearching = enable;
}

bool QImageWidget::subDirectorySearching() const
{
    return m_subDirectorySearching;
}

void QImageWidget::setPixmap( const QPixmap &pixmap )
{
    m_currentPixmap = pixmap;
    updatePixmap();
}
//---------------------------------------------------------------------------

// getters
QPixmap QImageWidget::currentPixmap() const
{
    return m_currentPixmap;
}
//---------------------------------------------------------------------------

QString QImageWidget::currentPixmapPath() const
{
    return m_currentPixmapPath;
}

int QImageWidget::currentPixmapIndex() const
{
    return m_currentPixmapIndex;
}

int QImageWidget::pixmapsCount() const
{
    return m_pixmapsPaths.size();
}

//---------------------------------------------------------------------------

void QImageWidget::updatePixmapByIndex()
{
    m_currentPixmapPath = m_pixmapsPaths.at( m_currentPixmapIndex );
    m_currentPixmap = QPixmap( m_currentPixmapPath );

    updatePixmap();
}
//---------------------------------------------------------------------------

void QImageWidget::updatePixmap()
{
    qDebug() << Q_FUNC_INFO  << trUtf8( "Update pixmap." );

    if ( m_currentPixmap.isNull() ) {
        qWarning() << Q_FUNC_INFO << trUtf8( "Null pixmap." );
        return;
    }

    m_customGraphicsView->scene()->clear();

    m_graphicsPixmapItem = m_customGraphicsView->scene()->addPixmap( m_currentPixmap );
    m_graphicsPixmapItem->setTransformationMode( Qt::SmoothTransformation );

    m_customGraphicsView->scene()->setSceneRect( m_currentPixmap.rect() );

    fillSize();

    // pixmaps signals
    emit currentPixmapChanged( m_currentPixmap );
    emit currentPixmapChangedBool( true );
    emit currentPixmapPathChanged( m_currentPixmapPath );
    emit pixmapAvailable( true );
    emit cropped( false );

    // false pixmap changed
    setCurrentPixmapModified( false );

    // check go operations enabled
    if ( m_currentPixmapIndex == 0  ) {       // if 1-st item
        if ( m_pixmapsPaths.size() > 1 ) {  // and if not the only one
            emit goFirstAvailable( false );
            emit goPreviousAvailable( m_endlessScrollEnabled );
            emit goNextAvailable( true );
            emit goLastAvailable( true );
        } else {    // if only one
            emit goFirstAvailable( false );
            emit goPreviousAvailable( false );
            emit goNextAvailable( false );
            emit goLastAvailable( false );
        }
    } else if ( m_currentPixmapIndex > 0 && m_currentPixmapIndex < m_pixmapsPaths.size() - 1 ) {    // if beetween 1-st and last
        emit goFirstAvailable( true );
        emit goPreviousAvailable( true );
        emit goNextAvailable( true );
        emit goLastAvailable( true );
    } else if ( m_currentPixmapIndex == m_pixmapsPaths.size() - 1 ) {   // if last
        emit goFirstAvailable( true );
        emit goPreviousAvailable( true );
        emit goNextAvailable( m_endlessScrollEnabled );
        emit goLastAvailable( false );
    } else {
        qWarning() << Q_FUNC_INFO << trUtf8( "The Thing That Should Not Be..." );
    }

    // previews
    if ( m_pixmapsPathsBefore != m_pixmapsPaths ) {
        m_pixmapsPathsBefore = m_pixmapsPaths;

        //        createPreviews();
        m_previewThread->setPreviewsList( m_pixmapsPaths );
        m_previewThread->start();
    }
}

//---------------------------------------------------------------------------

QString QImageWidget::standartPixmapsDirectory()
{
    return QStandardPaths::standardLocations( QStandardPaths::PicturesLocation ).at( 0 );
}
//---------------------------------------------------------------------------

bool QImageWidget::isCurrentPixmapModified() const
{
    return m_isCurrentPixmapModified;
}
//! [2]

//---------------------------------------------------------------------------

//! [3]

void QImageWidget::setEndlessScrollEnabled( const bool &value )
{
    m_endlessScrollEnabled = value;
}

bool QImageWidget::endlessScrollEnabled() const
{
    return m_endlessScrollEnabled;
}

void QImageWidget::goFirst()
{
    if ( m_pixmapsPaths.isEmpty() ) {
        qWarning() << trUtf8( "No files." );
        return;
    }

    m_currentPixmapIndex = 0;
    if ( m_previewVisible ) {
        m_previewWidget->setCurrentRow( m_currentPixmapIndex );
    } else {
        updatePixmapByIndex();
    }
}

//---------------------------------------------------------------------------

void QImageWidget::goPrevious()
{
    if ( m_pixmapsPaths.isEmpty() ) {
        qWarning() << trUtf8( "No files." );
        return;
    }

    if ( m_currentPixmapIndex > 0 ) {
        m_currentPixmapIndex--;
        if ( m_currentPixmapIndex == 0 ) {
            goFirst();
            return;
        }
    } else if ( m_currentPixmapIndex == 0 ) {
        if ( m_endlessScrollEnabled) {
            goLast();
            return;
        }
    } else {
        qWarning() << trUtf8( "The Thing That Should Not Be..." );
        return;
    }

    if ( m_previewVisible ) {
        m_previewWidget->setCurrentRow( m_currentPixmapIndex );
    } else {
        updatePixmapByIndex();
    }
}

//---------------------------------------------------------------------------

void QImageWidget::goNext()
{
    if ( m_pixmapsPaths.isEmpty() ) {
        qWarning() << trUtf8( "No files." );
        return;
    }

    if ( m_currentPixmapIndex < m_pixmapsPaths.size() - 1 ) {
        m_currentPixmapIndex++;
        if ( m_currentPixmapIndex == m_pixmapsPaths.size() - 1 ) {
            goLast();
            return;
        }
    } else if ( m_currentPixmapIndex == m_pixmapsPaths.size() - 1 ) {
        if ( m_endlessScrollEnabled ) {
            goFirst();
            return;
        }
    } else {
        qWarning() << trUtf8( "The Thing That Should Not Be..." );
        return;
    }

    if ( m_previewVisible ) {
        m_previewWidget->setCurrentRow( m_currentPixmapIndex );
    } else {
        updatePixmapByIndex();
    }
}

//---------------------------------------------------------------------------

void QImageWidget::goLast()
{
    if ( m_pixmapsPaths.isEmpty() ) {
        qWarning() << trUtf8( "No files." );
        return;
    }

    m_currentPixmapIndex = m_pixmapsPaths.size() - 1;
    if ( m_previewVisible ) {
        m_previewWidget->setCurrentRow( m_currentPixmapIndex );
    } else {
        updatePixmapByIndex();
    }
}
//! [3]

//---------------------------------------------------------------------------

//! [4]
void QImageWidget::setScaleFactor( const double &factor )
{
    m_scaleFactor = factor;
}

//---------------------------------------------------------------------------

double QImageWidget::scaleFactor() const
{
    return m_scaleFactor;
}

void QImageWidget::setScaleOnResize( const bool &scale )
{
    m_scaleOnResize = scale;
}

//---------------------------------------------------------------------------

bool QImageWidget::scaleOnResize() const
{
    return m_scaleOnResize;
}

//---------------------------------------------------------------------------

void QImageWidget::zoomIn()
{
    m_customGraphicsView->zoomIn();
}

//---------------------------------------------------------------------------

void QImageWidget::zoomOut()
{
    m_customGraphicsView->zoomOut();
}

//---------------------------------------------------------------------------

void QImageWidget::fillSize()
{
    m_customGraphicsView->fitInView( m_customGraphicsView->scene()->sceneRect(),
                                     Qt::KeepAspectRatio );
}

//---------------------------------------------------------------------------

void QImageWidget::fullSize()
{
    m_customGraphicsView->fitInView( m_customGraphicsView->rect(),
                                     Qt::KeepAspectRatio );
}

//---------------------------------------------------------------------------

void QImageWidget::resizeEvent( QResizeEvent *event )
{
    if ( m_scaleOnResize ) {
        if ( m_timerId ) {
            killTimer( m_timerId );
            m_timerId = 0;
        }

        m_timerId = startTimer( 100 );
    }

    event->accept();
}

void QImageWidget::timerEvent( QTimerEvent *event )
{
    qDebug() << Q_FUNC_INFO <<  trUtf8( "Fill size." );
    fillSize();

    killTimer( m_timerId );
    m_timerId = 0;

    event->accept();
}

void QImageWidget::mouseDoubleClickEvent( QMouseEvent *event )
{
    if ( m_scaled ) {
        fillSize();
        m_scaled = false;
    } else {
        //        QPointF p = event->pos();
        fullSize();
        //        // move to cursor pos
        //        m_customGraphicsView->horizontalScrollBar()->setValue( mapToGlobal( p.toPoint() ).x() );
        //        m_customGraphicsView->verticalScrollBar()->setValue( mapToGlobal( p.toPoint() ).y() );

        //        qDebug() << m_customGraphicsView->horizontalScrollBar()->maximum()
        //                 << mapToGlobal( p.toPoint() ).x();

        m_scaled = true;
    }

    event->accept();
}

//! [4]

//---------------------------------------------------------------------------

//! [5]
void QImageWidget::copy()
{
    QApplication::clipboard()->setPixmap( m_currentPixmap );
}

//---------------------------------------------------------------------------

//void QImageWidget::cut()
//{
//    // todo: undo, redo

//    copy();

//    m_customGraphicsView->scene()->removeItem( m_graphicsPixmapItem );
//}

//---------------------------------------------------------------------------

void QImageWidget::paste()
{
    QUndoCommand *pasteCommand = new QPasteCommand( this,
                                                    m_currentPixmap,
                                                    QApplication::clipboard()->pixmap() );
    m_undoStack->push( pasteCommand );

    setUndoRedoAvailable();
}

//---------------------------------------------------------------------------

void QImageWidget::checkPasteAvailable()
{
    emit pasteAvailable( !QApplication::clipboard()->pixmap().isNull() );
}

//! [5]

//---------------------------------------------------------------------------

//! [6]
void QImageWidget::undo()
{
    m_undoStack->undo();

    setUndoRedoAvailable();
}

//---------------------------------------------------------------------------

void QImageWidget::redo()
{
    m_undoStack->redo();

    setUndoRedoAvailable();
}

//---------------------------------------------------------------------------

bool QImageWidget::isUndoAvailable() const
{
    return m_undoStack->canUndo();
}

//---------------------------------------------------------------------------

bool QImageWidget::isRedoAvailable() const
{
    return m_undoStack->canRedo();
}

//---------------------------------------------------------------------------

void QImageWidget::setUndoRedoAvailable()
{
    emit undoAvailable( m_undoStack->canUndo() );
    emit redoAvailable( m_undoStack->canRedo() );

    setCurrentPixmapModified( m_undoStack->canUndo() );
}

//! [6]

//---------------------------------------------------------------------------

//! [7]
void QImageWidget::rotateLeft()
{
    QUndoCommand *rotateCommand = new QRotateCommand( this,
                                                      m_currentPixmap,
                                                      QRotateCommand::Left );
    m_undoStack->push( rotateCommand );

    setUndoRedoAvailable();
}

//---------------------------------------------------------------------------

void QImageWidget::rotateRight()
{
    QUndoCommand *rotateCommand = new QRotateCommand( this,
                                                      m_currentPixmap,
                                                      QRotateCommand::Right );
    m_undoStack->push( rotateCommand );

    setUndoRedoAvailable();
}

void QImageWidget::setCurrentPixmapModified( const bool &changed )
{
    qDebug() << Q_FUNC_INFO  << trUtf8( "Pixmap changed: " ) << changed;
    m_isCurrentPixmapModified = changed;
    emit currentPixmapModified( m_isCurrentPixmapModified );
}

//---------------------------------------------------------------------------

void QImageWidget::crop()
{
    if ( m_currentPixmap.isNull() ) {
        qWarning() << trUtf8( "No pixmap." );
        return;
    }

    qDebug() << Q_FUNC_INFO  << trUtf8( "Start cropping.");

    m_customGraphicsView->startSelection();
}

//---------------------------------------------------------------------------

void QImageWidget::getSelection( const QRect &rect )
{
    qDebug() << Q_FUNC_INFO  << trUtf8( "Cropped." );

    emit cropped( true );

    QUndoCommand *cropCommand = new QCropCommand( this,
                                                  m_currentPixmap,
                                                  m_currentPixmap.copy( rect ) );
    m_undoStack->push( cropCommand );

    setUndoRedoAvailable();
}

//---------------------------------------------------------------------------

void QImageWidget::remove()
{
    if ( m_currentPixmapPath.isEmpty() ) {
        qWarning() << Q_FUNC_INFO << trUtf8( "No image or wrong path." );
        return;
    }

    if ( m_showRemoveDialog ) {
        QMessageBox::StandardButtons button =  QMessageBox::question(this,
                                                                     trUtf8( "Delete" ),
                                                                     trUtf8( "Do you realy wanna delete this file?" ),
                                                                     QMessageBox::Cancel | QMessageBox::Apply );
        if ( button == QMessageBox::Cancel ) {
            return;
        } else if ( button == QMessageBox::Apply ) {
            deleteImage( m_currentPixmapPath );
        }
    } else {
        deleteImage( m_currentPixmapPath );
    }

    if ( m_pixmapsPaths.size() == 0 ) {
        m_currentPixmap = QPixmap();
        m_currentPixmapPath = QString();

        m_currentPixmapIndex = 0;
        m_customGraphicsView->scene()->clear();

        emit currentPixmapChanged( QPixmap() );
        emit currentPixmapChangedBool( true );
        emit currentPixmapPathChanged( QString() );
        emit pixmapAvailable( false );
    }
}

void QImageWidget::deleteImage( const QString &path )
{
    QFile::remove( path );
    m_pixmapsPaths.removeAt( m_currentPixmapIndex );
    // make this for do not update previews
    m_pixmapsPathsBefore = m_pixmapsPaths;

    // remove item from preview widget
    m_previewWidget->takeItem( m_currentPixmapIndex );
    if ( m_currentPixmapIndex >= 1 ) {
        goPrevious();
        m_previewWidget->currentRowChanged( m_currentPixmapIndex );
    }
}

void QImageWidget::setShowRemoveDialog( const bool &show )
{
    m_showRemoveDialog = show;
}

bool QImageWidget::showRemoveDialog() const
{
    return m_showRemoveDialog;
}

bool QImageWidget::setAsWallpaper()
{
    if ( m_currentPixmapPath.isNull() ) {
        qWarning() << Q_FUNC_INFO << trUtf8( "Null path." );
        return false;
    }

#ifdef Q_OS_WIN
    LPWSTR wallpaper = ( wchar_t * ) m_currentPixmapPath.toStdWString().c_str();
    bool m_result = SystemParametersInfo( SPI_SETDESKWALLPAPER, 0, wallpaper, SPIF_UPDATEINIFILE );

    QApplication::beep();

    if ( m_result ) {
        return true;
    } else {
        qWarning() << Q_FUNC_INFO << trUtf8( " Windows wallpaper settings error. " );
        return false;
    }
#endif

    return true;
}

bool QImageWidget::save()
{
    if ( !gotPath() ) {
        qWarning() << Q_FUNC_INFO << trUtf8( "Empty path" );
        QMessageBox::critical( this,
                               trUtf8( "Error" ),
                               trUtf8( "Cannot save. No image or wrong path." ) );
        return false;
    }

    if ( m_currentPixmap.save( m_currentPixmapPath ) ) {
        setCurrentPixmapModified( false );
        m_undoStack->clear();
        setUndoRedoAvailable();
        return true;
    }

    return false;
}

bool QImageWidget::saveAs()
{
    QString m_newPath = QFileDialog::getSaveFileName( this,
                                                      trUtf8( "Save" ),
                                                      m_currentPixmapPath,
                                                      m_filters );
    if ( m_newPath.isEmpty() ) {
        qWarning() << Q_FUNC_INFO << trUtf8( "Empty path" );
        return false;
    }

    m_currentPixmapPath = m_newPath;
    if ( m_currentPixmap.save( m_newPath ) ) {
        setCurrentPixmapModified( false );
        m_undoStack->clear();
        setUndoRedoAvailable();
        m_currentPixmap = QPixmap( m_newPath );
        updatePixmap();
        return true;
    }

    return false;
}
//! [7]

//! [8]
void QImageWidget::setPreviewVisible( const bool &show )
{
    m_previewVisible = show;
    if ( m_previewVisible ) {
        m_previewWidget->setCurrentRow( m_currentPixmapIndex );
    }

    m_previewWidget->setVisible( m_previewVisible );

    m_timerId = startTimer( 100 ); // resize timer
}

bool QImageWidget::previewVisible() const
{
    return m_previewVisible;
}

void QImageWidget::updatePreviewsForCurrentDirectory()
{
    m_pixmapsPathsBefore.clear();
    setPixmapsDirectory( QFileInfo( m_currentPixmapPath ).absolutePath() );
}

void QImageWidget::updatePreviewForStartingDirectory()
{
    m_pixmapsPathsBefore.clear();
    setPixmapsDirectory( m_startedDirectoryPath );
}

void QImageWidget::setPreviewPixmapSize( const QSize &size )
{
    m_previewPixmapSize = size;
    m_previewWidget->setMaximumWidth( m_previewPixmapSize.width() + 50 );
}

QSize QImageWidget::previewPixmapSize() const
{
    return m_previewPixmapSize;
}

void QImageWidget::currentPreviewChanged( const int &index )
{
    if ( index < 0 ) {
        qWarning() << Q_FUNC_INFO << trUtf8( "Wrong index" );
        return;
    }

    if ( m_currentPixmapIndex != index ) {
        m_currentPixmapIndex = index;
    }

    updatePixmapByIndex();
}

//! [8]

//! [9]
void QImageWidget::showInfomarion()
{
    if ( m_currentPixmapPath.isNull() ) {
        qWarning() << Q_FUNC_INFO << trUtf8( "No image or wrong path." );
        return;
    }

    QString f_color = "green";
    QString styled = trUtf8( "<font color=%1><b>%2</b></font>%3<br>");
    m_infoBox->setWindowTitle( currentPixmapFileName() );
    m_infoBox->setText( styled.arg( f_color, trUtf8( "Path: " ), currentPixmapPath() ) +
                        styled.arg( f_color, trUtf8( "File name: " ), currentPixmapFileName() ) +
                        styled.arg( f_color, trUtf8( "Created: " ), currentPixmapCreated() ) +
                        styled.arg( f_color, trUtf8( "<b>Last modified: </b>"), currentPixmapLastModified() ) +
                        styled.arg( f_color, trUtf8( "<b>Type: </b>" ), currentPixmapType() ) +
                        styled.arg( f_color, trUtf8( "<b>Size: </b>" ), QString::number( currentPixmapSizeMB() ) + trUtf8( " MB" ) ) +
                        styled.arg( f_color, trUtf8( "<b>Resolution: </b>" ), QString::number( currentPixmapWidth() ) +
                                    " : " + QString::number( currentPixmapHeight() ) ) );
    m_infoBox->show();
}



//! [9]

//! [10]
void QImageWidget::setContexActions( QList<QAction *> contexActions )
{
    m_contexActions = contexActions;
}

QList < QAction * > QImageWidget::contexActions()
{
    return m_contexActions;
}

void QImageWidget::contextMenuEvent( QContextMenuEvent *event )
{
    if ( m_contexActions.isEmpty() ) { return; }
    QMenu m_contexMenu;
    m_contexMenu.addActions( m_contexActions );
    m_contexMenu.exec( event->globalPos() );
}
//! [10]

//! [11]
void QImageWidget::appendNewPreview( const QString &path, QPixmap pixmap, const int &index )
{
    // check if update previews
    if ( index == 0 ) {
        m_previewWidget->clear();
    }

    // create new list item widget and widgets with name and preview
    QListWidgetItem *m_listWidgetItem = new QListWidgetItem( m_previewWidget  );
    QWidget *m_listWidgetItemWidget = new QWidget( this ); // crazy name?
    QVBoxLayout *m_listWidgetItemLayout = new QVBoxLayout( m_listWidgetItemWidget );
    m_listWidgetItemWidget->setLayout( m_listWidgetItemLayout );
    QLabel *m_previewName = new QLabel( QFileInfo( path ).fileName(), this );
    QLabel *m_previewIcon = new QLabel( this );

    QFont m_previewNameFont = m_previewName->font();
    m_previewNameFont.setPointSize( 7 );
    m_previewName->setFont( m_previewNameFont );

    m_previewIcon->setPixmap( pixmap.scaled( m_previewPixmapSize,
                                             Qt::KeepAspectRatioByExpanding,
                                             Qt::SmoothTransformation ) );
    m_listWidgetItem->setSizeHint( QSize ( m_previewPixmapSize.width() + 10,
                                           m_previewPixmapSize.height() + 20 ) );

    m_listWidgetItemLayout->addWidget( m_previewName );
    m_listWidgetItemLayout->addWidget( m_previewIcon );

    m_previewWidget->addItem( m_listWidgetItem );
    m_previewWidget->setItemWidget( m_listWidgetItem, m_listWidgetItemWidget );

    // check if last preview. we have to stop
    if ( m_pixmapsPaths.size() - 1 == index ) {
        m_previewThread->exit();
    }
}

//! [11]

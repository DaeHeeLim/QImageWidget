#ifndef QImageWidget_H
#define QImageWidget_H

#include <QObject>
#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QResizeEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QRubberBand>
#include <QScrollBar>
#include <QApplication>
#include <QClipboard>
#include <QUndoStack>
#include <QDirIterator>
#include <QStandardPaths>
#include <QLabel>
#include <QHBoxLayout>
#include <QSizeGrip>
#include <QGraphicsDropShadowEffect>
#include <QTextEdit>
#include <QMessageBox>
#include <QListWidget>
#include <QSplitter>
#include <QDate>
#include <QFileDialog>
#include <QContextMenuEvent>
#include <QAction>
#include <QMenu>
#include <QtCore>
#include <QThread>
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

class QPreviewThread;

//!--------------------------------------------------------------------
//!--------------------------------------------------------------------
//! RESIZABLE RUBBER BAND
class QResizableRubberBand : public QWidget
{
    Q_OBJECT

public:
    explicit QResizableRubberBand( QWidget *parent = 0 );
    ~QResizableRubberBand() {}

signals:
    void doubleClicked();

private:
    QRubberBand *rubberBand;

    QLabel *labelSize;

private:
    // resize
    void resizeEvent( QResizeEvent *event );

    // move rubber band
    void mousePressEvent( QMouseEvent *event );
    void mouseMoveEvent( QMouseEvent *event );
    void mouseReleaseEvent( QMouseEvent *event );

    QPoint rubberBandOffset;
    bool rubberBandMove;

    // need to know it for ending drawing rect on double clicked
    void mouseDoubleClickEvent( QMouseEvent *event );
};

//!--------------------------------------------------------------------
//!--------------------------------------------------------------------
//! QCUSTOM GRAPHICS VIEW
class QCustomGraphicsView : public QGraphicsView
{

    Q_OBJECT
    //! SIGNALS
signals:
    //! [2] SELECTION
    void selectedRect( const QRect & );
    void selectedDone();
    //! [2]

    //! PUBLIC METHODS
public:
    //! [1]
    explicit QCustomGraphicsView( QWidget *parent = 0 );
    ~QCustomGraphicsView();
    //! [1]

    //! [2] SELECTION
    void startSelection();
    void stopSelection();
    //! [2]

    //! PUBLIC SLOTS
public slots:
    //! [3]
    void zoomIn();
    void zoomOut();
    //! [3]


    //! PRIVATE SLOTS
private slots:
    void getRect(); // get rect from selector

    //! PRIVATE METHODS
private:
    //! [2]
    void mousePressEvent( QMouseEvent *event );
    void mouseMoveEvent( QMouseEvent *event );
    void mouseReleaseEvent( QMouseEvent *event );
    void keyReleaseEvent( QKeyEvent *event );
    //! [2]

    //! [3]
    void wheelEvent( QWheelEvent *event );
    //    void mouseDoubleClickEvent( QMouseEvent *event );
    //! [3]

    //! PRIVATE FIELDS
private:
    //! [1] MAIN
    QGraphicsScene *m_scene;
    QString m_currentPixmapPath;
    //! [1]

    //! [2] SELECTION
    QResizableRubberBand *m_selector;
    bool m_selection;
    QPoint m_selectionStartPosition;
    QPixmap m_pixmap;
    //! [2]

    //! [3]
    bool m_scaled;
    //! [3]
};


//!--------------------------------------------------------------------
//!--------------------------------------------------------------------
//! QIMAGE WIDGET
class QImageWidget : public QWidget
{
    Q_OBJECT

    //! PUBLIC METHODS
public:
    explicit QImageWidget( QWidget *parent = 0);
    ~QImageWidget();

    //! [1] MAIN WIDGETS
    QListWidget *previewWidget() { return m_previewWidget; }
    QCustomGraphicsView *customGraphicsView() { return m_customGraphicsView; }
    //! [1]

    //! [2] PIXMAP
    static QString standartPixmapsDirectory();

    bool isPixmapChanged() const;
    //! [2]

    //! SIGNALS
signals:
    //! [2] PIXMAP SIGNALS
    void currentPixmapChanged( const QPixmap & );
    void currentPixmapChangedBool( const bool & );
    void currentPixmapPathChanged( const QString & );

    void pixmapAvailable( const bool & );
    void pixmapChanged( const bool & );
    //! [2]

    //! [3] CONTROL SIGNALS
    void goFirstAvailable( const bool & );
    void goPreviousAvailable( const bool & );
    void goNextAvailable( const bool & );
    void goLastAvailable( const bool & );
    //! [3]

    //! [5] EDIT SIGNALS
    void pasteAvailable( const bool & );
    //! [5]

    //! [6] UNDO, REDO SIGNALS
    void undoAvailable( const bool & );
    void redoAvailable( const bool & );
    //! [6]

    //! [7]
    void cropped( const bool &c );
    //! [7]


    //! PUBLIC SLOTS
public slots:
    //! [2] SET PIXMAPS
    // setter
    void setPixmapsPaths( const QStringList &paths );
    void setPixmapsDirectory( const QString &dirPath );
    void setPixmapPathWithDirectorySearching( const QString &path );

    void setSubDirectorySearching( const bool &enable );
    bool subDirectorySearching() const;

    void setPixmap( const QPixmap &pixmap );

    // getters
    QPixmap currentPixmap() const;
    QString currentPixmapPath() const;

    int currentPixmapIndex() const;
    int pixmapsCount() const;
    //! [2]

    //! [3] CONTROL
    void setEndlessScrollEnabled( const bool &value );
    bool endlessScrollEnabled() const;

    void goFirst();
    void goPrevious();
    void goNext();
    void goLast();
    //! [3]

    //! [4] SCALE, ZOOM ( SIZE ) SETTINGS
    void setScaleFactor( const double &factor );
    double scaleFactor() const;

    void setScaleOnResize( const bool &scale );
    bool scaleOnResize() const;

    void zoomIn();
    void zoomOut();
    void fillSize();
    void fullSize();
    //! [4]

    //! [5] EDIT
    void copy();
    void cut();
    void paste();
    //! [5]

    //! [6] UNDO, REDO
    void undo();
    void redo();

    bool isUndoAvailable() const;
    bool isRedoAvailable() const;
    //! [6]

    //! [7] OPERATIONS
    // rotate
    void rotateLeft();
    void rotateRight();
    void setPixmapChanged( const bool &changed );
    void crop();
    void remove();
    void setShowRemoveDialog( const bool &show );
    bool showRemoveDialog() const;
    bool setAsWallpaper();
    bool save();
    bool saveAs();
    //! [7]

    //! [8] PREVIEW
    void setPreviewVisible( const bool &show );
    bool previewVisible() const;

    void updatePreviewsForCurrentDirectory();
    void updatePreviewForStartingDirectory();

    void setPreviewPixmapSize( const QSize &size );
    QSize previewPixmapSize() const;

    //! [8]

    //! [9] INFORMATION
    void showInfomarion();

    QString currentPixmapFileName() const {
        if ( gotPath() ) {
            return QFileInfo( m_currentPixmapPath ).fileName();
        }
        return QString();
    }

    QString currentPixmapCreated() const {
        if ( gotPath() ) {
            return QFileInfo ( m_currentPixmapPath ).created().toString( "dd MMMM yyyy" );
        }
        return QString();
    }

    QString currentPixmapLastModified() const {
        if ( gotPath() ) {
            return QFileInfo ( m_currentPixmapPath ).lastModified().toString( "dd MMMM yyyy" );
        }
        return QString();
    }

    QString currentPixmapType() const {
        if ( gotPath() ) {
            return  QFileInfo ( m_currentPixmapPath ).suffix().toUpper();
        }
        return QString();
    }

    float currentPixmapSize() const {
        if ( gotPath() ) {
            return QFileInfo ( m_currentPixmapPath ).size();
        }
        return -1;
    }

    float currentPixmapSizeMB() const {
        if ( gotPath() ) {
            return QFileInfo ( m_currentPixmapPath ).size() / ( 1024 * 1024 );
        }
        return -1;
    }

    float currentPixmapSizeKB() const {
        if ( gotPath() ) {
            return QFileInfo ( m_currentPixmapPath ).size() / ( 1024 );
        }
        return -1;
    }

    int currentPixmapWidth() const {
        if ( gotPixmap() ) {
            return m_currentPixmap.size().width();
        }
        return -1;
    }

    int currentPixmapHeight() const {
        if ( gotPixmap() ) {
            return m_currentPixmap.size().height();
        }
        return -1;
    }

    //! [9]

    //! [10] CONTEX MENU
    void setContexActions( QList < QAction * > contexActions );
    QList < QAction * > contexActions();
    //! [10]

    //! PRIVATE SIGNALS
private slots:
    //! [5] EDIT
    void checkPasteAvailable();
    //! [5]

    //! [7] OPERATIONS
    void getSelection( const QRect &rect );
    //! [7]

    //! [8] PREVIEW
    void currentPreviewChanged( const int &index );
    //! [8]

    //! [11]
    void appendNewPreview(const QString &path, QPixmap pixmap, const int &index);
    //! [11]

    //! PRIVATE FIELDS
private:
    //! [1] MAIN WIDGETS
    QCustomGraphicsView *m_customGraphicsView;
    QGraphicsScene *m_graphicsScene;
    QListWidget *m_previewWidget;
    QSplitter *m_splitter;
    //! [1]

    //! [2] PIXMAPS
    QStringList m_pixmapsPaths;
    QPixmap m_currentPixmap;
    QGraphicsPixmapItem *m_graphicsPixmapItem;
    QString m_currentPixmapPath;
    int m_currentPixmapIndex;
    bool m_isPixmapChanged;
    bool m_subDirectorySearching;
    QString m_filters; // png, jpg etc
    //! [2]

    //! [3] CONTROL
    bool m_endlessScrollEnabled;
    //! [3]

    //! [4] SCALE
    double m_scaleFactor;
    bool m_scaleOnResize;
    int m_timerId;

    bool m_scaled;
    //! [4]

    //! [6] UNDO, REDO
    QUndoStack *m_undoStack;
    //! [6]

    //! [7] OPERATIONS
    bool m_showRemoveDialog;
    bool m_moveToTrash;
    //! [7]

    //! [8] PREVIEW
    bool m_previewVisible;
    QStringList m_pixmapsPathsBefore;
    QSize m_previewPixmapSize;
    //! [8]

    //! [9] INFORMATION
    QMessageBox *m_infoBox;
    //! [9]

    //! [10]
    QList < QAction * > m_contexActions;
    //! [10]

    //! [11] PREVIEW THREAD
    QPreviewThread *m_previewThread;
    //! [11]

    //! PRIVATE METHODS
private:
    //! [2] PIXMAP
    void updatePixmapByIndex();
    void updatePixmap();

    QStringList searchDirectory(const QString &dirPath );

    bool gotPaths() const {
        return !m_pixmapsPaths.isEmpty();
    }

    bool gotPath() const {
        return !m_currentPixmapPath.isEmpty();
    }

    bool gotPixmap() const {
        return !m_currentPixmap.isNull();
    }

    //! [2]

    //! [4] SCALE
    void resizeEvent( QResizeEvent *event );
    void timerEvent( QTimerEvent *event );

    void mouseDoubleClickEvent( QMouseEvent *event );
    //! [4]

    //! [6] UNDO, REDO
    void setUndoRedoAvailable();
    //! [6]

    //! [7] OPERATIONS
    void deleteImage( const QString &path );
    //! [7]

    //! [8] PREVIEW
    void createPreviews();
    QString m_startedDirectoryPath;
    //! [8]

    //! [10] CONTEX MENU
    void contextMenuEvent( QContextMenuEvent *event );
    //! [10]
};

//!--------------------------------------------------------------------
//!--------------------------------------------------------------------
//! UNDO
//! CROP COMMAND
class QCropCommand : public QUndoCommand
{
public:
    explicit QCropCommand( QImageWidget *imageWidget,
                           const QPixmap &originalPixmap,
                           const QPixmap &croppedPixmap,
                           QUndoCommand *parent = 0 )
        : QUndoCommand( parent ) {
        m_imageWidget = imageWidget;
        m_originalPixmap = originalPixmap;
        m_croppedPixmap = croppedPixmap;
    }

    ~QCropCommand() {}

    void undo() {
        m_imageWidget->setPixmap( m_originalPixmap );
    }

    void redo() {
        m_imageWidget->setPixmap( m_croppedPixmap );
    }

private:
    QImageWidget *m_imageWidget;
    QPixmap m_originalPixmap;
    QPixmap m_croppedPixmap;
};

//! PASTE COMMAND
class QPasteCommand : public QUndoCommand
{
public:
    explicit QPasteCommand( QImageWidget *imageWidget,
                            const QPixmap &originalPixmap,
                            const QPixmap &pastedPixmap,
                            QUndoCommand *parent = 0 )
        : QUndoCommand ( parent ) {
        m_imageWidget = imageWidget;
        m_originalPixmap = originalPixmap;
        m_pastedPixmap = pastedPixmap;
    }

    ~QPasteCommand() {}

    void undo() {
        m_imageWidget->setPixmap( m_originalPixmap );
    }

    void redo() {
        m_imageWidget->setPixmap( m_pastedPixmap );
        m_imageWidget->setPixmapChanged( true );
    }

private:
    QImageWidget *m_imageWidget;
    QPixmap m_originalPixmap;
    QPixmap m_pastedPixmap;

};

//! ROTATE COMMAND
class QRotateCommand : public QUndoCommand
{

public:
    enum Direction { Left, Right };

    explicit QRotateCommand( QImageWidget *imageWidget,
                             const QPixmap &originalPixmap,
                             const Direction &direction,
                             QUndoCommand *parent = 0 )
        : QUndoCommand ( parent ) {
        m_imageWidget = imageWidget;
        m_originalPixmap = originalPixmap;
        m_direction = direction;
    }

    void undo() {
        if ( m_direction == Right ) {
            m_originalPixmap = m_originalPixmap.transformed( QMatrix().rotate( 90 ) );
        } else {
            m_originalPixmap = m_originalPixmap.transformed( QMatrix().rotate( -90 ) );
        }

        m_imageWidget->setPixmap( m_originalPixmap );
    }

    void redo() {
        if ( m_direction == Right ) {
            m_originalPixmap = m_originalPixmap.transformed( QMatrix().rotate( -90 ) );
        } else {
            m_originalPixmap = m_originalPixmap.transformed( QMatrix().rotate( 90 ) );
        }

        m_imageWidget->setPixmap( m_originalPixmap );
        m_imageWidget->setPixmapChanged( true );
    }

private:
    QImageWidget *m_imageWidget;
    QPixmap m_originalPixmap;
    Direction m_direction;

};

//!--------------------------------------------------------------------
//!--------------------------------------------------------------------
//! PREVIEW THREAD
class QPreviewThread : public QThread
{
    Q_OBJECT

signals:
    void previewWidgetReady( QListWidget * );
    void appendNewPreview( const QString &, QPixmap, const int & );

public:
    explicit QPreviewThread( QObject *parent )
        : QThread( parent ) {

    }

    ~QPreviewThread() {

    }

    void run() {
        for ( int i = 0; i < m_previewsList.size(); ++i ) {
            QPixmap pixmap = QPixmap( m_previewsList.at( i ) );
            emit appendNewPreview( m_previewsList.at(i), pixmap, i );
        }
    }

    void setPreviewsList( const QStringList & list) {
        m_previewsList = list;
    }

private:
    QStringList m_previewsList;

};
#endif // QImageWidget_H

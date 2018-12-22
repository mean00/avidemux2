
class dummyGLWidget : public QOpenGLWidget
{
public:
    dummyGLWidget(QWidget *parent);
    ~dummyGLWidget();
    void initializeGL();
    //void resizeGL(int w, int h);
    void paintGL();
};

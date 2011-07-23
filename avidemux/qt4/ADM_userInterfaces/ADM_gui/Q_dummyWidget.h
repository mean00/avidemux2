
class dummyGLWidget : public QGLWidget
{
public:
    dummyGLWidget(QWidget *parent,QGLWidget *sharing=NULL);
    ~dummyGLWidget();
    void initializeGL();
    //void resizeGL(int w, int h);
    void paintGL();
    

private:
  
};

#include "glyuvwidget.h"
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QDebug>
#define VERTEXIN 0
#define TEXTUREIN 1

GLYuvWidget::GLYuvWidget(QWidget *parent):
    QOpenGLWidget(parent)
{
}

GLYuvWidget::~GLYuvWidget()
{
    makeCurrent();
    vbo.destroy();
    textureY->destroy();
    textureU->destroy();
    textureV->destroy();
    doneCurrent();
}

void GLYuvWidget::slotShowYuv(uchar *ptr, uint width, uint height)
{
    yuvPtr = ptr;
    videoW = width;
    videoH = height;
    update();	//调用update将执行paintEvent函数
}

void GLYuvWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);

    static const GLfloat vertices[]{
        //顶点坐标
        -1.0f,-1.0f,
        -1.0f,+1.0f,
        +1.0f,+1.0f,
        +1.0f,-1.0f,
        //纹理坐标
        0.0f,1.0f,
        0.0f,0.0f,
        1.0f,0.0f,
        1.0f,1.0f,
    };

    vbo.create();
    vbo.bind();
    vbo.allocate(vertices,sizeof(vertices));
    //初始化顶点着色器对象
    QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex,this);
    //顶点着色器源码
    const char *vsrc =
   "attribute vec4 vertexIn; \
    attribute vec2 textureIn; \
    varying vec2 textureOut;  \
    void main(void)           \
    {                         \
        gl_Position = vertexIn; \
        textureOut = textureIn; \
    }";
    vshader->compileSourceCode(vsrc);	//编译顶点着色器程序

    //初始化片段着色器 功能gpu中yuv转换成rgb
    QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment,this);
    //片段着色器源码
    const char *fsrc =
    #if defined(WIN32)		//windows下opengl es 需要加上float这句话
        "#ifdef GL_ES\n"
        "precision mediump float;\n"
        "#endif\n"
    #endif
    "varying vec2 textureOut; \
    uniform sampler2D tex_y; \
    uniform sampler2D tex_u; \
    uniform sampler2D tex_v; \
    void main(void) \
    { \
        vec3 yuv; \
        vec3 rgb; \
        yuv.x = texture2D(tex_y, textureOut).r; \
        yuv.y = texture2D(tex_u, textureOut).r - 0.5; \
        yuv.z = texture2D(tex_v, textureOut).r - 0.5; \
        rgb = mat3( 1,       1,         1, \
                    0,       -0.39465,  2.03211, \
                    1.13983, -0.58060,  0) * yuv; \
        gl_FragColor = vec4(rgb, 1); \
    }";
    fshader->compileSourceCode(fsrc);	//将glsl源码送入编译器编译着色器程序
    //用于绘制矩形
    program = new QOpenGLShaderProgram(this);
    program->addShader(vshader);	//将顶点着色器添加到程序容器
    program->addShader(fshader);	//将片段着色器添加到程序容器
    //绑定属性vertexIn到指定位置ATTRIB_VERTEX,该属性在顶点着色源码其中有声明
    program->bindAttributeLocation("vertexIn",VERTEXIN);
    //绑定属性textureIn到指定位置ATTRIB_TEXTURE,该属性在顶点着色源码其中有声明
    program->bindAttributeLocation("textureIn",TEXTUREIN);
    program->link();	//链接所有所有添入到的着色器程序
    program->bind();	 //激活所有链接

    program->enableAttributeArray(VERTEXIN);
    program->enableAttributeArray(TEXTUREIN);
    program->setAttributeBuffer(VERTEXIN,GL_FLOAT,0,2,2*sizeof(GLfloat));
    program->setAttributeBuffer(TEXTUREIN,GL_FLOAT,8*sizeof(GLfloat),2,2*sizeof(GLfloat));

    //读取着色器中的数据变量tex_y, tex_u, tex_v的位置,这些变量的声明可以在片段着色器源码中可以看到
    textureUniformY = program->uniformLocation("tex_y");
    textureUniformU = program->uniformLocation("tex_u");
    textureUniformV = program->uniformLocation("tex_v");

    //分别创建y,u,v纹理对象
    textureY = new QOpenGLTexture(QOpenGLTexture::Target2D);
    textureU = new QOpenGLTexture(QOpenGLTexture::Target2D);
    textureV = new QOpenGLTexture(QOpenGLTexture::Target2D);
    textureY->create();
    textureU->create();
    textureV->create();

    idY = textureY->textureId();	//获取返回y分量的纹理索引值
    idU = textureU->textureId();	//获取返回u分量的纹理索引值
    idV = textureV->textureId();	//获取返回v分量的纹理索引值
    glClearColor(0.0,0.0,0.0,0.0);	//设置背景色为黑色
}

void GLYuvWidget::paintGL()
{
    //------- 加载y数据纹理 -------
    glActiveTexture(GL_TEXTURE0);  //激活纹理单元GL_TEXTURE0,系统里面的
    glBindTexture(GL_TEXTURE_2D,idY); //绑定y分量纹理对象id到激活的纹理单元
    //使用内存中的数据创建真正的y分量纹理数据
    glTexImage2D(GL_TEXTURE_2D,0,GL_RED,videoW,videoH,0,GL_RED,GL_UNSIGNED_BYTE,yuvPtr);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //------- 加载u数据纹理 -------
    glActiveTexture(GL_TEXTURE1); //激活纹理单元GL_TEXTURE1
    glBindTexture(GL_TEXTURE_2D,idU);
    //使用内存中的数据创建真正的u分量纹理数据
    glTexImage2D(GL_TEXTURE_2D,0,GL_RED,videoW >> 1, videoH >> 1,0,GL_RED,GL_UNSIGNED_BYTE,yuvPtr + videoW * videoH);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //------- 加载v数据纹理 -------
    glActiveTexture(GL_TEXTURE2); //激活纹理单元GL_TEXTURE2
    glBindTexture(GL_TEXTURE_2D,idV);
    //使用内存中的数据创建真正的v分量纹理数据
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, videoW >> 1, videoH >> 1, 0, GL_RED, GL_UNSIGNED_BYTE, yuvPtr+videoW*videoH*5/4);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //指定y纹理要使用新值,只能用0,1,2等表示纹理单元的索引，这是opengl不人性化的地方
    glUniform1i(textureUniformY, 0);	//0对应纹理单元GL_TEXTURE0,//指定y纹理要使用新值
    glUniform1i(textureUniformU, 1);	//1对应纹理单元GL_TEXTURE1,//指定u纹理要使用新值
    glUniform1i(textureUniformV, 2);	//2对应纹理的单元GL_TEXTURE2,指定v纹理要使用新值

    //使用顶点数组方式绘制图形
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

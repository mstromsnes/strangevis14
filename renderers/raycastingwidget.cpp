#include "raycastingwidget.h"

#include "../geometry.h"

RayCastingWidget::RayCastingWidget(RenderProperties initialRenderProperties,
                                   std::shared_ptr<ITextureStore> textureStore,
                                   QWidget* parent, Qt::WindowFlags f)
    : QOpenGLWidget(parent, f), m_textureStore{textureStore},
      m_transferFunctionName{initialRenderProperties.transferFunction},
      m_clippingPlaneEquation{initialRenderProperties.clippingPlaneEquation},
      m_projectionMode{initialRenderProperties.projectionMode}
{
    m_modelViewMatrix.setToIdentity();
    m_modelViewMatrix.translate(0.0, 0.0, -2.0 * sqrt(3.0));
    zoomCamera(initialRenderProperties.zoomFactor);

    connect(&m_textureStore->volume(), &Volume::dimensionsChanged, this,
            &RayCastingWidget::updateBoxScalingMatrix);
    connect(&m_textureStore->volume(), &Volume::volumeLoaded, this,
            [this]() { update(); });
}

void RayCastingWidget::rotateCamera(qreal angle, QVector3D axis)
{
    QMatrix4x4 inverseModelViewMatrix = m_modelViewMatrix.inverted();
    QVector4D transformedAxis = inverseModelViewMatrix * QVector4D(axis, 0.0f);
    m_modelViewMatrix.rotate(qRadiansToDegrees(angle),
                             transformedAxis.toVector3D());
    update();
}
void RayCastingWidget::zoomCamera(float zoomFactor)
{
    m_modelViewMatrix.scale(zoomFactor);
    update();
}

void RayCastingWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glEnable(GL_CLIP_DISTANCE0);
    // initialize geometry
    Geometry::instance();

    if (!m_cubeProgram.addShaderFromSourceFile(QOpenGLShader::Vertex,
                                               ":shaders/cube-vs.glsl"))
        qDebug() << "Could not load vertex shader!";

    if (!m_cubeProgram.addShaderFromSourceFile(QOpenGLShader::Fragment,
                                               ":shaders/cube-fs.glsl"))
        qDebug() << "Could not load fragment shader!";

    if (!m_cubeProgram.link())
        qDebug() << "Could not link shader program!";
}

void RayCastingWidget::resizeGL(int w, int h)
{
    qreal aspectRatio = static_cast<qreal>(w) / static_cast<qreal>(h);

    m_projectionMatrix.setToIdentity();
    if (m_projectionMode == Projection::Perspective)
        m_projectionMatrix.perspective(m_fov, aspectRatio, m_nearPlane,
                                       m_farPlane);
    if (m_projectionMode == Projection::Orthographic)
        m_projectionMatrix.ortho(-aspectRatio, aspectRatio, -1, 1,
                                 m_nearPlane, m_farPlane);
}

void RayCastingWidget::paintGL()
{
    int location = -1;

    glClearColor(0.95f, 0.95f, 0.95f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    QMatrix4x4 modelViewProjectionMatrix =
        m_projectionMatrix * m_modelViewMatrix * m_boxScalingMatrix;

    m_cubeProgram.bind();
    location = m_cubeProgram.uniformLocation("clippingPlaneEquation");
    m_cubeProgram.setUniformValue(location, m_clippingPlaneEquation);
    location = m_cubeProgram.uniformLocation("modelViewProjectionMatrix");
    m_cubeProgram.setUniformValue(location, modelViewProjectionMatrix);
    location = m_cubeProgram.uniformLocation("gradientMethod");
    m_cubeProgram.setUniformValue(location, static_cast<int>(m_gradientMethod));

    auto [width, height, depth] = m_textureStore->volume().getDimensions();
    location = m_cubeProgram.uniformLocation("width");
    m_cubeProgram.setUniformValue(location, static_cast<int>(width));
    location = m_cubeProgram.uniformLocation("height");
    m_cubeProgram.setUniformValue(location, static_cast<int>(height));
    location = m_cubeProgram.uniformLocation("depth");
    m_cubeProgram.setUniformValue(location, static_cast<int>(depth));

    glActiveTexture(GL_TEXTURE0);
    m_cubeProgram.setUniformValue("volumeTexture", 0);
    m_textureStore->volume().bind();

    // Transfer Texture
    glActiveTexture(GL_TEXTURE1);
    m_cubeProgram.setUniformValue("transferFunction", 1);
    m_textureStore->transferFunction().bind();

    Geometry::instance().bindCube();

    location = m_cubeProgram.attributeLocation("vertexPosition");
    m_cubeProgram.enableAttributeArray(location);
    m_cubeProgram.setAttributeBuffer(location, GL_FLOAT, 0, 3,
                                     sizeof(QVector3D));

    Geometry::instance().drawCube();
    glActiveTexture(GL_TEXTURE0);

    m_textureStore->transferFunction().release();
    m_textureStore->volume().release();
    m_cubeProgram.release();
}

void RayCastingWidget::updateBoxScalingMatrix(QVector3D dims)
{
    m_boxScalingMatrix.setToIdentity();
    auto maxDim = std::max(dims.x(), std::max(dims.y(), dims.z()));
    if (maxDim)
    {
        m_boxScalingMatrix.scale(dims / maxDim);
    }
    update();
}

void RayCastingWidget::updateClippingPlane(QVector4D clippingPlaneEquation)
{
    m_clippingPlaneEquation = clippingPlaneEquation;
    update();
}

void RayCastingWidget::changeGradientMethod(GradientMethod method)
{
    m_gradientMethod = method;
    update();
}

void RayCastingWidget::changeTransferFunction(QString transferFunction)
{
    m_transferFunctionName = transferFunction;
    update();
}

void RayCastingWidget::changeProjectionMode(Projection mode)
{
    m_projectionMode = mode;
    update();
}
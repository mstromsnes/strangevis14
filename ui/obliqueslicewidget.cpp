#include "obliqueslicewidget.h"

#include "../geometry.h"

#include <QVector3D>

ObliqueSliceWidget::ObliqueSliceWidget(
    std::shared_ptr<Environment> env,
    std::shared_ptr<SharedProperties> properties, QWidget* parent,
    Qt::WindowFlags f)
    : QOpenGLWidget(parent, f), m_environment(env), m_properties{properties},
      m_cubePlaneIntersection(m_properties->clippingPlane().plane()),
      m_prevRotation{0}, m_verticalFlipped{false}, m_horizontalFlipped{false}
{
    m_modelViewMatrix.scale(1 / sqrt(3.0));
}

void ObliqueSliceWidget::initializeGL()
{
    initializeOpenGLFunctions();
    // initialize geometry
    Geometry::instance();

    if (!m_sliceProgram.addShaderFromSourceFile(
            QOpenGLShader::Vertex, ":/shaders/shaders/slice-vs.glsl"))
        qDebug() << "Could not load vertex shader!";

    if (!m_sliceProgram.addShaderFromSourceFile(
            QOpenGLShader::Fragment, ":/shaders/shaders/slice-fs.glsl"))
        qDebug() << "Could not load fragment shader!";

    if (!m_sliceProgram.link())
        qDebug() << "Could not link shader program!";
    auto updateObliqueSlice = [this]() {
        Geometry::instance().allocateObliqueSlice(m_cubePlaneIntersection);
    };
    connect(&m_properties.get()->clippingPlane(),
            &ClippingPlaneProperties::clippingPlaneChanged,
            [this, updateObliqueSlice](const Plane& plane) {
                m_cubePlaneIntersection.changePlane(plane);
                updateObliqueSlice();
                update();
            });
    updateObliqueSlice();
}

void ObliqueSliceWidget::paintGL()
{

    Geometry::instance().allocateObliqueSlice(m_cubePlaneIntersection);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_sliceProgram.bind();

    m_sliceProgram.setUniformValue("modelViewMatrix", m_modelViewMatrix);

    glActiveTexture(GL_TEXTURE0);
    m_sliceProgram.setUniformValue("volumeTexture", 0);
    m_environment->volume()->bind();

    Geometry::instance().bindObliqueSliceVertex();
    {
        int location = m_sliceProgram.attributeLocation("vertexPosition");
        m_sliceProgram.enableAttributeArray(location);
        m_sliceProgram.setAttributeBuffer(location, GL_FLOAT, 0, 2,
                                          sizeof(QVector2D));
    }

    Geometry::instance().bindObliqueSliceTexCoord();
    {
        int location = m_sliceProgram.attributeLocation("texCoord");
        m_sliceProgram.enableAttributeArray(location);
        m_sliceProgram.setAttributeBuffer(location, GL_FLOAT, 0, 3,
                                          sizeof(QVector3D));
    }

    Geometry::instance().drawObliqueSlice();

    glActiveTexture(GL_TEXTURE0);
    m_environment->volume()->release();

    m_sliceProgram.release();
}

void ObliqueSliceWidget::rotate(float degrees)
{
    QVector3D zAxis(0, 0, 1);
    m_modelViewMatrix.rotate(m_prevRotation, zAxis);
    m_modelViewMatrix.rotate(-degrees, zAxis);
    m_prevRotation = degrees;
    update();
}
void ObliqueSliceWidget::flipHorizontal(bool flip)
{
    int scale = flip != m_horizontalFlipped ? -1 : 1;
    float prevRotation = m_prevRotation;
    rotate(0);
    m_modelViewMatrix.scale(scale, 1);
    m_horizontalFlipped = flip;
    rotate(prevRotation);
}
void ObliqueSliceWidget::flipVertical(bool flip)
{
    int scale = flip != m_verticalFlipped ? -1 : 1;
    float prevRotation = m_prevRotation;
    rotate(0);
    m_modelViewMatrix.scale(1, scale);
    m_verticalFlipped = flip;
    rotate(prevRotation);
}
ObliqueSliceRotationWidget::ObliqueSliceRotationWidget(const std::shared_ptr<SharedProperties>& properties, QWidget* parent)
    : m_flipHorizontalCheckbox{tr("Flip Horizontal")},
      m_flipVerticalCheckbox{tr("Flip Vertical")}, m_layout{this}, m_parameterWidget(properties, this)
{
    m_dial.setRange(0, 359);
    connect(&m_dial, &QDial::valueChanged, this,
            &ObliqueSliceRotationWidget::angleChanged);

    connect(&m_flipHorizontalCheckbox, &QCheckBox::stateChanged, this,
            &ObliqueSliceRotationWidget::flipHorizontal);
    connect(&m_flipVerticalCheckbox, &QCheckBox::stateChanged, this,
            &ObliqueSliceRotationWidget::flipVertical);
    QVBoxLayout* rotationLayout = new QVBoxLayout();
    rotationLayout->addWidget(&m_dial);
    QHBoxLayout* checkboxLayout = new QHBoxLayout();
    checkboxLayout->addWidget(&m_flipHorizontalCheckbox);
    checkboxLayout->addWidget(&m_flipVerticalCheckbox);
    rotationLayout->addLayout(checkboxLayout);
    m_layout.addLayout(rotationLayout);
    m_layout.addWidget(&m_parameterWidget);
}

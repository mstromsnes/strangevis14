#include "obliquesliceinteractor.h"

ObliqueSliceInteractor::ObliqueSliceInteractor(
    std::unique_ptr<ITextureStore>& textureStore,
    const std::shared_ptr<ISharedProperties> properties, QWidget* parent,
    Qt::WindowFlags f)
    : ObliqueSliceRenderWidget{textureStore, properties, parent, f},
      m_properties{properties}
{
    m_dial = new QDial(this);
    m_dial->setRange(0, 359);
    connect(m_dial, &QDial::valueChanged, this,
            &ObliqueSliceInteractor::rotate);
    placeDial();
    m_dial->show();
}

void ObliqueSliceInteractor::resizeEvent(QResizeEvent* event)
{
    placeDial();
    ObliqueSliceRenderWidget::resizeEvent(event);
}

void ObliqueSliceInteractor::placeDial()
{
    m_dial->setGeometry(geometry().width() - (DialSize + DialMargin),
                        geometry().height() - (DialSize + DialMargin), DialSize,
                        DialSize);
}

void ObliqueSliceInteractor::wheelEvent(QWheelEvent* event)
{
    QPoint deg = event->angleDelta();
    float zoomScale = 1.0;

    if (deg.y() < 0)
    {
        zoomScale = zoomScale / 1.1;
    }
    if (deg.y() > 0)
    {
        zoomScale = zoomScale * 1.1;
    }
    zoomCamera(zoomScale);
}

ObliqueSliceRotationWidget::ObliqueSliceRotationWidget(
    const std::shared_ptr<ISharedProperties>& properties, QWidget* parent)
    : QWidget(parent), m_flipHorizontalCheckbox{tr("Flip Horizontal")},
      m_flipVerticalCheckbox{tr("Flip Vertical")}, m_layout{this},
      m_parameterWidget(properties, this)
{
    connect(&m_flipHorizontalCheckbox, &QCheckBox::stateChanged, this,
            &ObliqueSliceRotationWidget::flipHorizontal);
    connect(&m_flipVerticalCheckbox, &QCheckBox::stateChanged, this,
            &ObliqueSliceRotationWidget::flipVertical);
    QHBoxLayout* checkboxLayout = new QHBoxLayout();
    checkboxLayout->addWidget(&m_flipHorizontalCheckbox);
    checkboxLayout->addWidget(&m_flipVerticalCheckbox);
    m_layout.addLayout(checkboxLayout);
    m_layout.addWidget(&m_parameterWidget);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
}

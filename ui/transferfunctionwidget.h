#ifndef TRANSFERFUNCTIONWIDGET_H
#define TRANSFERFUNCTIONWIDGET_H

#include "../transfertexture.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QOpenGLTexture>
#include <QSlider>
#include <QWidget>

class ISharedProperties;
/*
TODO:
Add DataPoints
Add Colormaps
Add more settings

*/
namespace tfn
{

class ColorMapSelector : public QComboBox
{
    Q_OBJECT
  public:
    ColorMapSelector(QWidget* parent, std::vector<QString> options);
};

class TransferWidget : public QWidget
{
    Q_OBJECT
  public:
    TransferWidget(const std::shared_ptr<ISharedProperties> properties, const std::shared_ptr<const IColorMapStore> colorMapStore,
                   QWidget* parent);
  public slots:
    void setSelectedColorMap(const QString& name);
  signals:
    void valueChanged(const QString& cmap);

  private:
    const std::shared_ptr<ISharedProperties> m_properties;
    const std::shared_ptr<const IColorMapStore> m_colorMapStore;
    QHBoxLayout* m_layout;
    QString m_selectedColorMap;
    ColorMapSelector* m_selector;
    std::vector<ColorMap> m_colorMaps;
};

} // namespace tfn
#endif // TRANSFERFUNCTIONWIDGET_H

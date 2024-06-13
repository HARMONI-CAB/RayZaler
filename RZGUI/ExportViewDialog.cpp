#include "ExportViewDialog.h"
#include <TopLevelModel.h>
#include "SessionTabWidget.h"
#include "SimulationSession.h"
#include "ui_ExportViewDialog.h"
#include "RZGUIGLWidget.h"
#include "GUIHelpers.h"
#include "ModelRenderer.h"
#include <QCursor>
#include <QFileDialog>
#include <QMessageBox>

ExportViewDialog::ExportViewDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ExportViewDialog)
{
  ui->setupUi(this);

  connectAll();
}

void
ExportViewDialog::connectAll()
{
  connect(
        ui->browseButton,
        SIGNAL(clicked(bool)),
        this,
        SLOT(onBrowse()));

  connect(
        ui->widthSpin,
        SIGNAL(valueChanged(int)),
        this,
        SLOT(onChangeWidthSpin()));

  connect(
        ui->heightSpin,
        SIGNAL(valueChanged(int)),
        this,
        SLOT(onChangeHeightSpin()));

  connect(
        ui->sameAsWindowButton,
        SIGNAL(clicked(bool)),
        this,
        SLOT(onResetSame()));

  connect(
        ui->buttonBox,
        SIGNAL(accepted()),
        this,
        SLOT(onSave()));

  connect(
        ui->buttonBox,
        SIGNAL(rejected()),
        this,
        SLOT(onCancel()));
}

void
ExportViewDialog::setSessionTabWidget(SessionTabWidget *widget)
{
  if (widget != m_sessionTab) {
    m_sessionTab = widget;
    m_glWidget = nullptr;

    QString path = widget->session()->path();
    QString suggestedPath = appendExtToPath(path, "png");

    setWindowTitle("Export view - " + widget->session()->fileName());

    m_glWidget = widget->glWidget();

    ui->pathEdit->setText(suggestedPath);

    m_nominalWidth  = m_glWidget->width();
    m_nominalHeight = m_glWidget->height();

    BLOCKSIG(ui->widthSpin,  setValue(m_glWidget->width()));
    BLOCKSIG(ui->heightSpin, setValue(m_glWidget->height()));
  }
}

ExportViewDialog::~ExportViewDialog()
{

  delete ui;
}

//
// Now, there is a couple of things that must be adjusted here. When
// the display resolution (given by the m_glWidget) and the offscreen
// renderer resolution are different, keeping the same absolute center
// makes the relative center change. We need to calculate the relative
// center as:
//
// RelCenterX = ViewCenterX / (zoom * width)
// RelCenterY = ViewCenterY / (zoom * width)
//
// Finally, we update the ViewCenterX,Y wrt the new RelCenterX,Y, leaving
// things properly framed
//


void
ExportViewDialog::renderAndSave()
{
  RZ::ModelRenderer *renderer = nullptr;
  RZ::Real relCenterX, relCenterY;
  auto view = m_glWidget->view();

  setCursor(Qt::CursorShape::WaitCursor);
  QCoreApplication::processEvents();

  relCenterX = view->center[0] / (view->zoomLevel * view->width);
  relCenterY = view->center[1] / (view->zoomLevel * view->width);

  renderer = RZ::ModelRenderer::fromOMModel(
        m_sessionTab->session()->topLevelModel(),
        SCAST(unsigned, ui->widthSpin->value()),
        SCAST(unsigned, ui->heightSpin->value()));

  renderer->setView(m_glWidget->view());

  // Recenter
  view = renderer->view();
  renderer->setCenter(
        relCenterX * view->zoomLevel * view->width,
        relCenterY * view->zoomLevel * view->width);

  renderer->render();
  renderer->savePNG(ui->pathEdit->text().toStdString().c_str());

  delete renderer;

  setCursor(Qt::CursorShape::ArrowCursor);
}

void
ExportViewDialog::adjustHeight(qreal ratio)
{
  int newHeight = ui->heightSpin->value();
  m_nominalHeight = m_nominalWidth * ratio;
  newHeight       = floor(m_nominalHeight);
  BLOCKSIG(ui->heightSpin, setValue(newHeight));
}

void
ExportViewDialog::adjustWidth(qreal ratio)
{
  int newWidth  = ui->widthSpin->value();
  m_nominalWidth = m_nominalHeight * ratio;
  newWidth       = floor(m_nominalWidth);
  BLOCKSIG(ui->widthSpin, setValue(newWidth));
}

///////////////////////////////// Slots ///////////////////////////////////////
void
ExportViewDialog::onChangeWidthSpin()
{
  qreal ratio;
  int newWidth  = ui->widthSpin->value();

  ratio = m_nominalHeight / m_nominalWidth;

  m_nominalWidth = m_nominalWidth - floor(m_nominalWidth) + newWidth;

  if (ui->lockButton->isChecked())
    adjustHeight(ratio);
}

void
ExportViewDialog::onChangeHeightSpin()
{
  qreal ratio;
  int newHeight = ui->heightSpin->value();

  ratio = m_nominalWidth / m_nominalHeight;

  m_nominalHeight = m_nominalHeight - floor(m_nominalHeight) + newHeight;

  if (ui->lockButton->isChecked())
    adjustWidth(ratio);
}

void
ExportViewDialog::onLockToggled()
{

}

void
ExportViewDialog::onResetSame()
{
  m_nominalWidth  = m_glWidget->width();
  m_nominalHeight = m_glWidget->height();

  BLOCKSIG(ui->widthSpin,  setValue(m_glWidget->width()));
  BLOCKSIG(ui->heightSpin, setValue(m_glWidget->height()));
}

void
ExportViewDialog::onSave()
{
  QFile file(ui->pathEdit->text());

  if (file.exists()) {
    auto answer = QMessageBox::question(
          this,
          "Overwrite output file",
          "The destination file already exists. Do you want to overwrite it?");

    if (answer != QMessageBox::Yes)
      return;
  }

  renderAndSave();
}

void
ExportViewDialog::onCancel()
{
  hide();
}

void
ExportViewDialog::onBrowse()
{
  QString path = QFileDialog::getSaveFileName(
        this,
        "Export view",
        QString(),
        ".png",
        nullptr,
        QFileDialog::Option::DontConfirmOverwrite);

  if (!path.isEmpty())
    ui->pathEdit->setText(path);
}

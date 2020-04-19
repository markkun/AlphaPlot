#include "Bar2D.h"

#include <gsl/gsl_histogram.h>
#include <gsl/gsl_vector.h>

#include "AxisRect2D.h"
#include "DataManager2D.h"
#include "ErrorBar2D.h"
#include "Table.h"
#include "core/Utilities.h"
#include "future/core/column/Column.h"
#include "future/lib/XmlStreamReader.h"
#include "future/lib/XmlStreamWriter.h"

Bar2D::Bar2D(Table *table, Column *xcol, Column *ycol, int from, int to,
             Axis2D *xAxis, Axis2D *yAxis, int stackposition)
    : QCPBars(xAxis, yAxis),
      barwidth_(1),
      xaxis_(xAxis),
      yaxis_(yAxis),
      bardata_(new DataBlockBar(table, xcol, ycol, from, to)),
      ishistogram_(false),
      table_(nullptr),
      column_(nullptr),
      from_(-1),
      to_(-1),
      xerrorbar_(nullptr),
      yerrorbar_(nullptr),
      xerroravailable_(false),
      yerroravailable_(false),
      picker_(Graph2DCommon::Picker::None),
      stackposition_(stackposition) {
  init();
  setSelectable(QCP::SelectionType::stSingleData);
  QColor color = Utilities::getRandColorGoldenRatio(Utilities::ColorPal::Dark);
  setstrokecolor_barplot(color);
  color.setAlpha(100);
  setfillcolor_barplot(color);
  setData(bardata_->data());
  if (bardata_ && bardata_->data()->size() > 1)
    setWidth(bardata_->data()->at(1)->mainKey() -
             bardata_->data()->at(0)->mainKey());
}

Bar2D::Bar2D(Table *table, Column *ycol, int from, int to, Axis2D *xAxis,
             Axis2D *yAxis)
    : QCPBars(xAxis, yAxis),
      barwidth_(1),
      xaxis_(xAxis),
      yaxis_(yAxis),
      ishistogram_(true),
      table_(table),
      column_(ycol),
      from_(from),
      to_(to),
      xerrorbar_(nullptr),
      yerrorbar_(nullptr),
      xerroravailable_(false),
      yerroravailable_(false),
      picker_(Graph2DCommon::Picker::None),
      auto_binning_(true),
      bin_size_(0),
      begin_(0),
      end_(0) {
  init();
  setSelectable(QCP::SelectionType::stSingleData);
  QColor color = Utilities::getRandColorGoldenRatio(Utilities::ColorPal::Dark);
  setstrokecolor_barplot(color);
  color.setAlpha(100);
  setfillcolor_barplot(color);
  setBarData(table_, column_, from_, to_);
}

void Bar2D::init() {
  layername_ = QString("<Bar2D>") +
      QDateTime::currentDateTime().toString("yyyy:MM:dd:hh:mm:ss:zzz");
  QThread::msleep(1);
  parentPlot()->addLayer(layername_, xaxis_->layer(), QCustomPlot::limBelow);
  setLayer(layername_);
  layer()->setMode(QCPLayer::LayerMode::lmBuffered);
}

Bar2D::~Bar2D() {
  if (!ishistogram_) delete bardata_;
  parentPlot()->removeLayer(layer());
}

void Bar2D::setXerrorBar(Table *table, Column *errorcol, int from, int to) {
  if (xerroravailable_ || ishistogram_) {
    qDebug() << "X error bar already defined or unsupported plot type";
    return;
  }
  xerrorbar_ = new ErrorBar2D(table, errorcol, from, to, xaxis_, yaxis_,
                              QCPErrorBars::ErrorType::etKeyError, this);
  parentPlot()->moveLayer(layer(), xerrorbar_->layer(),
                          QCustomPlot::LayerInsertMode::limAbove);
  xerroravailable_ = true;
  emit xaxis_->getaxisrect_axis()->ErrorBar2DCreated(xerrorbar_);
}

void Bar2D::setYerrorBar(Table *table, Column *errorcol, int from, int to) {
  if (yerroravailable_ || ishistogram_) {
    qDebug() << "Y error bar already defined or unsupported plot type";
    return;
  }
  yerrorbar_ = new ErrorBar2D(table, errorcol, from, to, xaxis_, yaxis_,
                              QCPErrorBars::ErrorType::etValueError, this);
  parentPlot()->moveLayer(layer(), yerrorbar_->layer(),
                          QCustomPlot::LayerInsertMode::limAbove);
  yerroravailable_ = true;
  emit yaxis_->getaxisrect_axis()->ErrorBar2DCreated(yerrorbar_);
}

void Bar2D::removeXerrorBar() {
  if (!xerroravailable_) return;

  parentPlot()->removePlottable(xerrorbar_);
  xerrorbar_ = nullptr;
  xerroravailable_ = false;
  emit xaxis_->getaxisrect_axis()->ErrorBar2DRemoved(
      xaxis_->getaxisrect_axis());
}

void Bar2D::removeYerrorBar() {
  if (!yerroravailable_) return;

  parentPlot()->removePlottable(yerrorbar_);
  yerrorbar_ = nullptr;
  yerroravailable_ = false;
  emit yaxis_->getaxisrect_axis()->ErrorBar2DRemoved(
      yaxis_->getaxisrect_axis());
}

Axis2D *Bar2D::getxaxis() const { return xaxis_; }

Axis2D *Bar2D::getyaxis() const { return yaxis_; }

Qt::PenStyle Bar2D::getstrokestyle_barplot() const { return pen().style(); }

QColor Bar2D::getstrokecolor_barplot() const { return pen().color(); }

double Bar2D::getstrokethickness_barplot() const { return pen().widthF(); }

QColor Bar2D::getfillcolor_barplot() const { return brush().color(); }

DataBlockBar *Bar2D::getdatablock_barplot() const { return bardata_; }

bool Bar2D::ishistogram_barplot() const { return ishistogram_; }

void Bar2D::setxaxis_barplot(Axis2D *axis, bool override) {
  if (!override)
    Q_ASSERT(axis->getorientation_axis() == Axis2D::AxisOreantation::Bottom ||
             axis->getorientation_axis() == Axis2D::AxisOreantation::Top);
  if (axis == getxaxis()) return;

  xaxis_ = axis;
  setKeyAxis(axis);
}

void Bar2D::setyaxis_barplot(Axis2D *axis, bool override) {
  if (!override)
    Q_ASSERT(axis->getorientation_axis() == Axis2D::AxisOreantation::Left ||
             axis->getorientation_axis() == Axis2D::AxisOreantation::Right);
  if (axis == getyaxis()) return;

  yaxis_ = axis;
  setValueAxis(axis);
}

void Bar2D::setstrokestyle_barplot(const Qt::PenStyle &style) {
  QPen p = pen();
  p.setStyle(style);
  setPen(p);
}

void Bar2D::setstrokecolor_barplot(const QColor &color) {
  QPen p = pen();
  p.setColor(color);
  setPen(p);
}

void Bar2D::setstrokethickness_barplot(const double value) {
  QPen p = pen();
  p.setWidthF(value);
  setPen(p);
}

void Bar2D::setfillcolor_barplot(const QColor &color) {
  QBrush b = brush();
  b.setColor(color);
  setBrush(b);
}

void Bar2D::setBarData(Table *table, Column *xcol, Column *ycol, int from,
                       int to) {
  bardata_->regenerateDataBlock(table, xcol, ycol, from, to);
  setData(bardata_->data());
}

void Bar2D::setBarData(Table *table, Column *col, int from, int to) {
  table_ = table;
  column_ = col;
  from_ = from;
  to_ = to;
  int d_end_row = to;
  int d_start_row = from;
  int d_bin_size = 0;
  double d_begin = 0.0;
  double d_end = 0.0;
  double d_mean = 0.0;
  double d_standard_deviation = 0.0;
  double d_min = 0.0;
  double d_max = 0.0;

  int r = abs(d_end_row - d_start_row) + 1;
  QVarLengthArray<double> Y(r);

  Column *y_col_ptr = col;
  int yColType = table->columnType(table->colIndex(col->name()));
  int size = 0;
  for (int row = d_start_row; row <= d_end_row && row < y_col_ptr->rowCount();
       row++) {
    if (!y_col_ptr->isInvalid(row)) {
      if (yColType == Table::Text) {
        QString yval = y_col_ptr->textAt(row);
        bool valid_data = true;
        Y[size] = QLocale().toDouble(yval, &valid_data);
        if (!valid_data) continue;
      } else
        Y[size] = y_col_ptr->valueAt(row);
      size++;
    }
  }

  if (size < 2 || (size == 2 && Y[0] == Y[1])) {  // non valid histogram
    double X[2];
    Y.resize(2);
    for (int i = 0; i < 2; i++) {
      Y[i] = 0;
      X[i] = 0;
    }
    return;
  }

  int n;
  gsl_histogram *h;
  if (auto_binning_) {
    n = 10;
    h = gsl_histogram_alloc(n);
    if (!h) return;

    gsl_vector *v = gsl_vector_alloc(size);
    for (int i = 0; i < size; i++) gsl_vector_set(v, i, Y[i]);

    double min, max;
    gsl_vector_minmax(v, &min, &max);
    gsl_vector_free(v);

    d_begin = floor(min);
    d_end = ceil(max);
    d_bin_size = (d_end - d_begin) / static_cast<double>(n);

    gsl_histogram_set_ranges_uniform(h, floor(min), ceil(max));
  } else {
    n = int((d_end - d_begin) / d_bin_size + 1);
    h = gsl_histogram_alloc(n);
    if (!h) return;

    double *range = new double[n + 2];
    for (int i = 0; i <= n + 1; i++) range[i] = d_begin + i * d_bin_size;

    gsl_histogram_set_ranges(h, range, n + 1);
    delete[] range;
  }

  for (int i = 0; i < size; i++) gsl_histogram_increment(h, Y[i]);

  double X[n];  // stores ranges (x) and bins (y)
  Y.resize(n);
  QSharedPointer<QCPBarsDataContainer> cont =
      QSharedPointer<QCPBarsDataContainer>(new QCPBarsDataContainer);
  for (int i = 0; i < n; i++) {
    QCPBarsData dat;
    Y[i] = gsl_histogram_get(h, i);
    dat.value = gsl_histogram_get(h, i);
    double lower, upper;
    gsl_histogram_get_range(h, i, &lower, &upper);
    X[i] = lower;
    dat.key = lower;
    cont.data()->add(dat);
  }

  setData(cont);

  d_mean = gsl_histogram_mean(h);
  d_standard_deviation = gsl_histogram_sigma(h);
  d_min = gsl_histogram_min_val(h);
  d_max = gsl_histogram_max_val(h);

  gsl_histogram_free(h);
  if (!cont.isNull() && cont.data()->size() > 1)
    setWidth(cont.data()->at(1)->mainKey() - cont.data()->at(0)->mainKey());
  bin_size_ = d_bin_size;
  begin_ = d_begin;
  end_ = d_end;
}

void Bar2D::setpicker_barplot(const Graph2DCommon::Picker picker) {
  picker_ = picker;
}

void Bar2D::save(XmlStreamWriter *xmlwriter, int xaxis, int yaxis) {
  xmlwriter->writeStartElement("bar");
  // axis
  (getxaxis()->getorientation_axis() == Axis2D::AxisOreantation::Top ||
   getxaxis()->getorientation_axis() == Axis2D::AxisOreantation::Bottom)
      ? xmlwriter->writeAttribute("orientation", "vertical")
      : xmlwriter->writeAttribute("orientation", "horizontal");
  xmlwriter->writeAttribute("xaxis", QString::number(xaxis));
  xmlwriter->writeAttribute("yaxis", QString::number(yaxis));
  xmlwriter->writeAttribute("legend", name());
  (ishistogram_) ? xmlwriter->writeAttribute("type", "histogram")
                 : xmlwriter->writeAttribute("type", "barxy");
  if (ishistogram_) {
    xmlwriter->writeAttribute("table", table_->name());
    xmlwriter->writeAttribute("column", column_->name());
  } else {
    xmlwriter->writeAttribute("table", bardata_->gettable()->name());
    xmlwriter->writeAttribute("xcolumn", bardata_->getxcolumn()->name());
    xmlwriter->writeAttribute("ycolumn", bardata_->getycolumn()->name());
    xmlwriter->writeAttribute("from", QString::number(bardata_->getfrom()));
    xmlwriter->writeAttribute("to", QString::number(bardata_->getto()));
    xmlwriter->writeAttribute("stackorder",
                              QString::number(getstackposition_barplot()));
  }
  xmlwriter->writeAttribute("stackgap", QString::number(stackingGap()));
  // error bar
  if (xerroravailable_) xerrorbar_->save(xmlwriter);
  if (yerroravailable_) yerrorbar_->save(xmlwriter);

  // line
  xmlwriter->writeStartElement("box");
  xmlwriter->writeAttribute("width", QString::number(width()));
  (antialiased()) ? xmlwriter->writeAttribute("antialias", "true")
                  : xmlwriter->writeAttribute("antialias", "false");
  (antialiasedFill()) ? xmlwriter->writeAttribute("antialiasfill", "true")
                      : xmlwriter->writeAttribute("antialiasfill", "false");
  xmlwriter->writePen(pen());
  xmlwriter->writeBrush(brush());
  xmlwriter->writeEndElement();
  xmlwriter->writeEndElement();
}

bool Bar2D::load(XmlStreamReader *xmlreader) {
  bool ok;
  while (!xmlreader->atEnd()) {
    if (xmlreader->isEndElement() && xmlreader->name() == "bar") break;

    if (xmlreader->isStartElement() && xmlreader->name() == "box") {
      // width
      double w = xmlreader->readAttributeDouble("width", &ok);
      (ok) ? setWidth(w)
           : xmlreader->raiseWarning(tr("Bar2D width property setting error"));

      // antialias
      bool ant = xmlreader->readAttributeBool("antialias", &ok);
      (ok) ? setAntialiased(ant)
           : xmlreader->raiseWarning(
                 tr("Bar2D antialias property setting error"));

      // antialias fill
      bool antfill = xmlreader->readAttributeBool("antialiasfill", &ok);
      (ok) ? setAntialiasedFill(antfill)
           : xmlreader->raiseWarning(
                 tr("Bar2D antialias fill property setting error"));

      // pen property
      while (!xmlreader->atEnd()) {
        xmlreader->readNext();
        if (xmlreader->isEndElement() && xmlreader->name() == "pen") break;
        // pen
        if (xmlreader->isStartElement() && xmlreader->name() == "pen") {
          QPen strokep = xmlreader->readPen(&ok);
          if (ok) {
            setPen(strokep);
          } else
            xmlreader->raiseWarning(tr("Bar2D pen property setting error"));
        }
      }

      // brush property
      while (!xmlreader->atEnd()) {
        xmlreader->readNext();
        if (xmlreader->isEndElement() && xmlreader->name() == "brush") break;
        // brush
        if (xmlreader->isStartElement() && xmlreader->name() == "brush") {
          QBrush b = xmlreader->readBrush(&ok);
          if (ok) {
            setBrush(b);
          } else
            xmlreader->raiseWarning(tr("Bar2D brush property setting error"));
        }
      }
    }

    xmlreader->readNext();
  }

  return !xmlreader->hasError();
}

void Bar2D::mousePressEvent(QMouseEvent *event, const QVariant &details) {
  if (event->button() == Qt::LeftButton) {
    switch (picker_) {
      case Graph2DCommon::Picker::None:
        break;
      case Graph2DCommon::Picker::DataPoint:
        datapicker(event, details);
        break;
      case Graph2DCommon::Picker::DataGraph:
        graphpicker(event, details);
        break;
      case Graph2DCommon::Picker::DataMove:
        movepicker(event, details);
        break;
      case Graph2DCommon::Picker::DataRemove:
        removepicker(event, details);
        break;
    }
  }
  QCPBars::mousePressEvent(event, details);
}

void Bar2D::datapicker(QMouseEvent *, const QVariant &details) {
  QCPBarsDataContainer::const_iterator it = data()->constEnd();
  QCPDataSelection dataPoints = details.value<QCPDataSelection>();
  if (dataPoints.dataPointCount() > 0) {
    dataPoints.dataRange();
    it = data()->at(dataPoints.dataRange().begin());
    QPointF point = coordsToPixels(it->mainKey(), it->mainValue());
    emit showtooltip(point, it->mainKey(), it->mainValue());
  }
}

void Bar2D::graphpicker(QMouseEvent *event, const QVariant &) {
  double xvalue, yvalue;
  pixelsToCoords(event->localPos(), xvalue, yvalue);
  emit showtooltip(event->localPos(), xvalue, yvalue);
}

void Bar2D::movepicker(QMouseEvent *event, const QVariant &details) {}

void Bar2D::removepicker(QMouseEvent *, const QVariant &details) {
  QCPBarsDataContainer::const_iterator it = data()->constEnd();
  QCPDataSelection dataPoints = details.value<QCPDataSelection>();
  if (dataPoints.dataPointCount() > 0) {
    dataPoints.dataRange();
    it = data()->at(dataPoints.dataRange().begin());
    bardata_->removedatafromtable(it->mainKey(), it->mainValue());
  }
}

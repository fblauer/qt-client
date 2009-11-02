/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#include "quoteList.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QVBoxLayout>

#include <parameter.h>

#include "xtreewidget.h"
#include "warehousegroup.h"
#include "quotecluster.h"
#include "xsqlquery.h"

quoteList::quoteList( QWidget* parent, const char* name, bool modal, Qt::WFlags fl ) : QDialog( parent, name, modal, fl )
{
  if ( !name )
    setObjectName( "quoteList" );

  _quheadid = -1;
  _custid = -1;

  setWindowTitle(tr("Quotes"));

  QVBoxLayout *mainLayout = new QVBoxLayout(this, 5, 5, "mainLayout"); 
  QVBoxLayout *warehouseLayout = new QVBoxLayout(0, 0, 0, "warehouseLayout"); 
  QHBoxLayout *topLayout = new QHBoxLayout( 0, 0, 7, "topLayout"); 
  QVBoxLayout *buttonsLayout = new QVBoxLayout(0, 0, 5, "buttonsLayout");
  QVBoxLayout *listLayout = new QVBoxLayout( 0, 0, 0, "listLayout"); 

  _warehouse = new WarehouseGroup(this, "_warehouse");
  warehouseLayout->addWidget(_warehouse);

  QSpacerItem* spacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Preferred);
  warehouseLayout->addItem(spacer);
  topLayout->addLayout(warehouseLayout);

  QSpacerItem* spacer_2 = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
  topLayout->addItem(spacer_2);

  _close = new QPushButton(tr("&Cancel"), this, "_close");
  buttonsLayout->addWidget(_close);

  _select = new QPushButton(tr("&Select"), this, "_select");
  _select->setEnabled( FALSE );
  _select->setDefault( TRUE );
  buttonsLayout->addWidget(_select);
  buttonsLayout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));
  topLayout->addLayout(buttonsLayout);
  mainLayout->addLayout(topLayout);

  QLabel *_quotesLit = new QLabel(tr("Quotes:"), this, "_quotesLit");
  listLayout->addWidget(_quotesLit);

  _qu = new XTreeWidget(this);
  _qu->setObjectName("_qu");
  listLayout->addWidget(_qu, 1);
  mainLayout->addLayout(listLayout, 1);

  resize( QSize(490, 390).expandedTo(minimumSizeHint()) );

  connect( _close, SIGNAL( clicked() ), this, SLOT( reject() ) );
  connect( _select, SIGNAL( clicked() ), this, SLOT( sSelect() ) );
  connect( _qu, SIGNAL( itemSelected(int) ), _select, SLOT( animateClick() ) );
  connect( _qu, SIGNAL( valid(bool) ), _select, SLOT( setEnabled(bool) ) );
  connect( _warehouse, SIGNAL(updated()), this, SLOT( sFillList() ) );

  _qu->addColumn(tr("Order #"), _orderColumn, Qt::AlignLeft,  true, "quhead_number");
  _qu->addColumn(tr("Customer"),          -1, Qt::AlignLeft,  true, "cust_name");
  _qu->addColumn(tr("Ordered"),  _dateColumn, Qt::AlignCenter,true, "quhead_quotedate");
  _qu->addColumn(tr("Scheduled"),_dateColumn, Qt::AlignCenter,true, "duedate");

  setTabOrder(_warehouse, _qu);
  setTabOrder(_qu, _select);
  setTabOrder(_select, _close);
  setTabOrder(_close, _warehouse);
  _warehouse->setFocus();
}

void quoteList::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("quhead_id", &valid);
  if (valid)
    _quheadid = param.toInt();
    
  param = pParams.value("quoteType", &valid);
  if (valid)
    _type = param.toInt();
    
  param = pParams.value("cust_id", &valid);
  if (valid)
    _custid = param.toInt();
 
  sFillList();
}

void quoteList::sSelect()
{
  done(_qu->id());
}

void quoteList::sFillList()
{
  QString sql;

  sql = "SELECT DISTINCT quhead_id, quhead_number, cust_name, quhead_quotedate,"
        "                MIN(quitem_scheddate) AS duedate "
        "FROM quhead, quitem, itemsite, cust "
        "WHERE ((quhead_cust_id=cust_id)"
        " AND (quitem_quhead_id=quhead_id)"
        " AND (quitem_itemsite_id=itemsite_id)";

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  if (_custid != -1)
    sql += " AND (quhead_cust_id=:cust_id)";

  sql += ") "
         "GROUP BY quhead_id, quhead_number, cust_name, quhead_quotedate "
         "ORDER BY quhead_number;";

  XSqlQuery q;
  q.prepare(sql);
  _warehouse->bindValue(q);
  q.bindValue(":cust_id", _custid);
  q.exec();
  _qu->populate(q, _quheadid);
}
/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "dspCustomerARHistory.h"

#include <QVariant>
#include <QStatusBar>
#include <QWorkspace>
#include <QMessageBox>
#include <QMenu>

#include <metasql.h>
#include "mqlutil.h"

#include <openreports.h>
#include "arOpenItem.h"

/*
 *  Constructs a dspCustomerARHistory as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspCustomerARHistory::dspCustomerARHistory(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_custhist, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*, QTreeWidgetItem *)));

  _custhist->setRootIsDecorated(TRUE);
  _custhist->addColumn(tr("Open"),      _dateColumn,     Qt::AlignCenter );
  _custhist->addColumn(tr("Doc. Type"), _itemColumn,     Qt::AlignCenter );
  _custhist->addColumn(tr("Doc. #"),    _orderColumn,    Qt::AlignRight  );
  _custhist->addColumn(tr("Doc. Date"), _dateColumn,     Qt::AlignCenter );
  _custhist->addColumn(tr("Due Date"),  _dateColumn,     Qt::AlignCenter );
  _custhist->addColumn(tr("Amount"),    _moneyColumn,    Qt::AlignRight  );
  _custhist->addColumn(tr("Balance"),   _moneyColumn,    Qt::AlignRight  );

  _cust->setFocus();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspCustomerARHistory::~dspCustomerARHistory()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspCustomerARHistory::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspCustomerARHistory::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("cust_id", &valid);
  if (valid)
    _cust->setId(param.toInt());

  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate());

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspCustomerARHistory::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  int menuItem;

  if (((XTreeWidgetItem *)pSelected)->id() != -1)
  {
    menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
    if (!_privileges->check("EditAROpenItem"))
      pMenu->setItemEnabled(menuItem, FALSE);

    pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
  }
}

void dspCustomerARHistory::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("aropen_id", _custhist->id());

  arOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspCustomerARHistory::sView()
{
  ParameterList params;
  params.append("mode", "view");

  if (_custhist->id() == -1)
    params.append("aropen_id", _custhist->altId());
  else
    params.append("aropen_id", _custhist->id());

  arOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspCustomerARHistory::sPrint()
{
  if(!checkParameters())
    return;

  ParameterList params;
  params.append("cust_id", _cust->id());
  _dates->appendValue(params);

  orReport report("CustomerARHistory", params);
  if (report.isValid())
      report.print();
  else
    report.reportError(this);
}

void dspCustomerARHistory::sFillList()
{
  _custhist->clear();

  if (!checkParameters())
    return;

  MetaSQLQuery mql = mqlLoad(":/ar/arHistory.mql");
  ParameterList params;
  _dates->appendValue(params);
  params.append("invoice", tr("Invoice"));
  params.append("zeroinvoice", tr("Zero Invoice"));
  params.append("creditMemo", tr("C/M"));
  params.append("debitMemo", tr("D/M"));
  params.append("check", tr("Check"));
  params.append("certifiedCheck", tr("Certified Check"));
  params.append("masterCard", tr("Master Card"));
  params.append("visa", tr("Visa"));
  params.append("americanExpress", tr("American Express"));
  params.append("discoverCard", tr("Discover Card"));
  params.append("otherCreditCard", tr("Other Credit Card"));
  params.append("cash", tr("Cash"));
  params.append("wireTransfer", tr("Wire Transfer"));
  params.append("cashdeposit", tr("Cash Deposit"));
  params.append("other", tr("Other"));
  params.append("cust_id", _cust->id());
  q = mql.toQuery(params);

  if (q.first())
  {
    XTreeWidgetItem *document = 0;
    XTreeWidgetItem *last = 0;
    do
    {
      if (q.value("type").toInt() == 1)
        last = document = new XTreeWidgetItem( _custhist, last,
                                               q.value("aropen_id").toInt(), q.value("applyid").toInt(),
                                               q.value("f_open"), q.value("documenttype"),
                                               q.value("docnumber"), q.value("f_docdate"),
                                               q.value("f_duedate"), q.value("f_amount"),
                                               q.value("f_balance") );
      else if (document)
        last = new XTreeWidgetItem( document,
                                    q.value("aropen_id").toInt(), q.value("applyid").toInt(),
                                    "", q.value("documenttype"),
                                    q.value("docnumber"), q.value("f_docdate"),
                                    "", q.value("f_amount") );
    }
    while (q.next());
  }
}

bool dspCustomerARHistory::checkParameters()
{
  if (!_cust->isValid())
  {
    if (isVisible())
    {
      QMessageBox::warning( this, tr("Enter Customer Number"),
                            tr("Please enter a valid Customer Number.") );
      _cust->setFocus();
    }
    return FALSE;
  }

  if (!_dates->startDate().isValid())
  {
    if (isVisible())
    {
      QMessageBox::warning( this, tr("Enter Start Date"),
                            tr("Please enter a valid Start Date.") );
      _dates->setFocus();
    }
    return FALSE;
  }

  if (!_dates->endDate().isValid())
  {
    if (isVisible())
    {
      QMessageBox::warning( this, tr("Enter End Date"),
                            tr("Please enter a valid End Date.") );
      _dates->setFocus();
    }
    return FALSE;
  }

  return TRUE;
}

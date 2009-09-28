  /*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef XTABLEWIDGET_H
#define XTABLEWIDGET_H

#include <QTableView>
#include <QMenu>

#include "xdatawidgetmapper.h"
#include "xsqltablemodel.h"
#include "widgets.h"

class XTUPLEWIDGETS_EXPORT XTableWidget : public QTableView
{
    Q_OBJECT
    Q_PROPERTY(QString         schemaName            READ schemaName           WRITE setSchemaName       )
    Q_PROPERTY(QString         tableName             READ tableName            WRITE setTableName        )
    Q_PROPERTY(int             primaryKeyCoulmns     READ primaryKeyColumns    WRITE setPrimaryKeyColumns)
    
    public:
      XTableWidget(QWidget *parent = 0);
      ~XTableWidget();

                  int     primaryKeyColumns() const { return _keyColumns; };
                  QString schemaName()        const { return _schemaName; };
                  QString tableName()         const { return _tableName;  };

                  bool    throwScriptException(const QString &message);

      Q_INVOKABLE      QString columnNameFromLogicalIndex(const int logicalIndex) const;
      Q_INVOKABLE         void setColumn(const QString &label, int width, int alignment, bool visible, const QString &colname);
      Q_INVOKABLE virtual void setColumnLocked(const QString &pColname, bool pLocked);
      Q_INVOKABLE virtual void setColumnLocked(const int      pColumn, bool pLocked);
      Q_INVOKABLE virtual void setColumnVisible(int, bool);
      Q_INVOKABLE         void setTable();
 //     Q_INVOKABLE XDataWidgetMapper *mapper()  { return _mapper;};
 //     Q_INVOKABLE XSqlTableModel    model()   { return static_cast<XSqlTableModel>(_mapper->model());};
      
    public slots:

      virtual int  rowCount();
      virtual int  rowCountVisible();
 //     virtual QString filter()            const { return _model.filter()   };
      virtual QVariant value(int row, int column);
      virtual QVariant selectedValue(int column); 
      virtual void insert();
      virtual void populate(int p);
      virtual void removeSelected();
      virtual void revertAll(); 
      virtual void save();
      virtual void select();
      virtual void setDataWidgetMap(XDataWidgetMapper* mapper);
 //     virtual void setFilter(QString filter)                  { _model.setFilter(filter);  };
      virtual void setModel(XSqlTableModel* model=0);
      virtual void setPrimaryKeyColumns(int p)                { _keyColumns = p;            };
      virtual void setSchemaName(QString p)                   { _schemaName = p;            };
      virtual void setTableName(QString p)                    { _tableName = p;             };
      virtual void setValue(int row, int column, QVariant value);
      virtual void sShowMenu(const QPoint &);

    signals:
      void  newModel(XSqlTableModel *model);
      void  rowSelected(int);
      void  valid(bool);
      void  saved();
      void  populateMenu(QMenu *, QModelIndex);
      
    protected:
      virtual void resizeEvent(QResizeEvent*);
      virtual void selectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
      virtual void setRelations();

    private slots:
      void popupMenuActionTriggered(QAction*);
      void sColumnSizeChanged(int, int, int);
      void sResetAllWidths();
      void sResetWidth();
      void sShowHeaderMenu(const QPoint &);
      void sToggleForgetfulness();

    private:
      QSqlDatabase        *_db;
      bool                 _forgetful;
      QMultiMap<QString,QString> _idMap;
      QSqlRecord           _idx;
      int                  _keyColumns;
      XDataWidgetMapper   *_mapper;
      QMenu               *_menu;
      XSqlTableModel       _model;
      int                  _resetWhichWidth;
      bool                 _resizingInProcess;
      QString              _schemaName;
      QItemSelectionModel *_selectModel;
      bool                 _settingsLoaded;
      QString              _tableName;
      QString              _windowName;

      struct ColumnProps
      { QString columnName;
        int     logicalIndex;
        int     defaultWidth;
        int     savedWidth;
        bool    stretchy;
        bool    locked;
        bool    visible;
        QString label;
        int     alignment;
        bool    fromSettings;
      };
      QMap<QString, ColumnProps*> _columnByName;

      QMap<QString, QString>    _fkeymap;
};

#endif
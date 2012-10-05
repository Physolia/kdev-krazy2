/*
 * This file is part of KDevelop Krazy2 Plugin.
 *
 * Copyright 2012 Daniel Calviño Sánchez <danxuliu@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "issuewidget.h"

#include <kdevplatform/interfaces/icore.h>
#include <kdevplatform/interfaces/idocumentcontroller.h>

#include "issue.h"
#include "issuemodel.h"

//public:

IssueWidget::IssueWidget(QWidget* parent /*= 0*/): QTableView(parent) {
    setSelectionBehavior(SelectRows);

    connect(this, SIGNAL(activated(QModelIndex)),
            this, SLOT(openDocumentForActivatedIssue(QModelIndex)));
}

//private slots:

void IssueWidget::openDocumentForActivatedIssue(const QModelIndex& index) const {
    if (!index.isValid()) {
        return;
    }

    const IssueModel* model = static_cast<IssueModel*>(QTableView::model());
    const Issue* issue = model->issueForIndex(index);

    KUrl url = issue->fileName();
    KTextEditor::Cursor line(issue->line() - 1, 0);

    KDevelop::ICore::self()->documentController()->openDocument(url, line);
}
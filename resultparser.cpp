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

#include "resultparser.h"

#include <QRegExp>

#include <KUrl>

#include "analysisresults.h"
#include "checker.h"
#include "issue.h"

//public:

ResultParser::ResultParser():
    m_analysisResults(0),
    m_checker(0),
    m_checkerBeingInitialized(0) {
}

void ResultParser::setAnalysisResults(AnalysisResults* analysisResults) {
    m_analysisResults = analysisResults;
}

void ResultParser::setWorkingDirectory(const QString& workingDirectory) {
    m_workingDirectory = workingDirectory;

    if (!m_workingDirectory.endsWith('/')) {
        m_workingDirectory.append('/');
    }
}

void ResultParser::parse(const QString& data) {
    m_xmlStreamReader.addData(data);

    while (!m_xmlStreamReader.atEnd()) {
        m_xmlStreamReader.readNext();

        if (isStartElement("file-type")) {
            processFileTypeStart();
        }
        if (isStartElement("check")) {
            processCheckStart();
        }
        if (isEndElement("explanation")) {
            processExplanationEnd();
        }
        if (isStartElement("file")) {
            processFileStart();
        }
        if (isEndElement("message")) {
            processMessageEnd();
        }
        if (isStartElement("line")) {
            processLineStart();
        }
        if (isEndElement("line")) {
            processLineEnd();
        }
        if (m_xmlStreamReader.isCharacters()) {
            m_text = m_xmlStreamReader.text().toString();
        }
    }
}

//private:

bool ResultParser::isStartElement(const QString& elementName) {
    return m_xmlStreamReader.isStartElement() &&
           m_xmlStreamReader.name() == elementName;
}

bool ResultParser::isEndElement(const QString& elementName) {
    return m_xmlStreamReader.isEndElement() &&
           m_xmlStreamReader.name() == elementName;
}

void ResultParser::processFileTypeStart() {
    m_checkerFileType = m_xmlStreamReader.attributes().value("value").toString();
}

void ResultParser::processCheckStart() {
    QRegExp regExp("(.*) \\[(.*)\\]\\.\\.\\.");
    regExp.indexIn(m_xmlStreamReader.attributes().value("desc").toString());
    QString checkerDescription = regExp.cap(1);
    QString checkerName = regExp.cap(2);

    m_checker = m_analysisResults->findChecker(m_checkerFileType, checkerName);
    m_checkerBeingInitialized = 0;

    if (!m_checker) {
        m_checkerBeingInitialized = new Checker();
        m_checkerBeingInitialized->setName(checkerName);
        m_checkerBeingInitialized->setDescription(checkerDescription);
        m_checkerBeingInitialized->setFileType(m_checkerFileType);

        m_analysisResults->addChecker(m_checkerBeingInitialized);

        m_checker = m_checkerBeingInitialized;
    }
}

void ResultParser::processExplanationEnd() {
    if (m_checkerBeingInitialized) {
        m_checkerBeingInitialized->setExplanation(m_text);
    }
}

void ResultParser::processFileStart() {
    QString fileName = m_xmlStreamReader.attributes().value("name").toString();

    //The working directory must end with a '/'
    //If fileName is absolute, or the working directory is empty, only fileName
    //is returned
    m_issueFileName = QUrl(m_workingDirectory).resolved(fileName).toString();
}

void ResultParser::processMessageEnd() {
    m_issueMessage = m_text;
}

void ResultParser::processLineStart() {
    if (!m_xmlStreamReader.attributes().hasAttribute("issue")) {
        return;
    }

    QString message = m_xmlStreamReader.attributes().value("issue").toString();

    QRegExp bracketsRegExp("\\[(.*)\\]");
    if (bracketsRegExp.indexIn(message) != -1) {
        message = bracketsRegExp.cap(1);
    }

    m_issueMessage = message;
}

void ResultParser::processLineEnd() {
    Issue* issue = new Issue();
    issue->setChecker(m_checker);
    issue->setFileName(m_issueFileName);
    issue->setMessage(m_issueMessage);
    issue->setLine(m_text.toInt());

    m_analysisResults->addIssue(issue);
}

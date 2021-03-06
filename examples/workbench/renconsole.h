//
// renconsole.cpp
// This file is part of Ren Garden
// Copyright (C) 2015 MetÆducation
//
// Ren Garden is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Ren Garden is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Ren Garden.  If not, see <http://www.gnu.org/licenses/>.
//
// See http://ren-garden.metaeducation.com for more information on this project
//

#ifndef RENCONSOLE_H
#define RENCONSOLE_H

#include <memory>
#include <unordered_map>

#include <QTabWidget>
#include <QMutex>
#include <QThread>

#include "rencpp/ren.hpp"

#include "replpad.h"
#include "renshell.h"
#include "renpackage.h"
#include "watchlist.h"

class MainWindow;

class RenConsole :
    public QTabWidget, public IReplPadHooks, public IReplPadSyntaxer
{
    Q_OBJECT

public:
    RenConsole (QWidget * parent = nullptr);
    ~RenConsole () override;

private:
    // Syntax highlighting hooks
    std::pair<int, int> rangeForWholeToken(
        QString buffer, int offset
    ) const override;

    std::pair<QString, int> autoComplete(
        QString const & text, int index, bool backwards
    ) const override;

private:
    RenShell shell;

public:
    ReplPad const & repl() const {
        return *qobject_cast<ReplPad *>(currentWidget());
    }
    ReplPad const & replFromIndex(int index) const {
        return *qobject_cast<ReplPad *>(widget(index));
    }

    ReplPad & repl() {
        return *qobject_cast<ReplPad *>(currentWidget());
    }
    ReplPad & replFromIndex(int index) {
        return *qobject_cast<ReplPad *>(widget(index));
    }

private:
    struct TabInfo {
        ren::Function dialect; // what dialect processor tab is running
        std::experimental::optional<ren::Tag> label; // label of the tab
        ren::Context context;
        WatchList * watchList;
    };

    std::unordered_map<ReplPad const *, TabInfo> tabinfo;

public:
    WatchList & watchList() {
        auto it = tabinfo.find(&repl());
        return *(it->second.watchList);
    }

signals:
    void switchWatchList(WatchList * watchList);

    void showDockRequested(WatchList * watchList);

    void hideDockRequested(WatchList * watchList);

protected:
    bool bannerPrinted;
    void printBanner();
    QString getPromptString() override;

protected:
    bool isReadyToModify(QKeyEvent * event) override;

private:
    ReplPad * evaluatingRepl;
    QThread workerThread;
    NulOStream nulOstream;

private:
    ren::Value consoleFunction;

    void escape() override;

private:
    // Experimental facility for writing the shell's output to a string

    ren::Value target;

    // You can set up a value that represents what the pending console
    // buffer will be.  It is evaluated after the command is done to
    // produce a string, and will be forgotten once used.  If there is
    // an error in the command, no buffer will be loaded.

    QString pendingBuffer;
    int pendingAnchor;
    int pendingPosition;

public slots:
    void handleResults(
        bool success,
        ren::Value const & result
    );
signals:
    // keep terminology from Qt sample
    void operate(
        ren::Value const & dialectValue,
        ren::Value const & contextValue,
        QString const & input,
        bool meta
    );
    void finishedEvaluation();
protected:
    void evaluate(QString const & input, bool meta) override;

    // Primitive "package management", which is just an experiment to get
    // some basic caching and updating started (a test case for figuring out
    // where to put local files and settings, etc.)
private:
    QSharedPointer<RenPackage> proposals;
    QSharedPointer<RenPackage> helpers;

signals:
    void reportStatus(QString const & str);

public:
    void createNewTab();
    void tryCloseTab(int index);
    void updateTabLabels();
};

#endif

/*
* Copyright (c) 2012 Research In Motion Limited.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "ScoreloopSampleCascades.hpp"

#include <bb/cascades/Application>
#include <bb/cascades/QmlDocument>
#include <bb/cascades/AbstractPane>
#include <bb/cascades/Window>
#include <QtDeclarative/QtDeclarative>

#include <scoreloop/scoreloopcore.h>
#include "ScoreloopDefines.hpp"

using namespace bb::cascades;

ScoreloopSampleCascades::ScoreloopSampleCascades(bb::cascades::Application *app)
: QObject(app)
{
	// We must initialize scoreloop only after the app has posted some window.
    // So, we show a splash screen and listen to the posted() signal of the mainWindow on which we
    // proceed with the main scene.
    bool connected = connect(app->mainWindow(), SIGNAL(posted()), this, SLOT(showMain()));
    Q_ASSERT(connected);
    Q_UNUSED(connected);

    QmlDocument *qml = QmlDocument::create("asset:///splash.qml").parent(this);
    AbstractPane *root = qml->createRootObject<AbstractPane>();
    app->setScene(root);
}

void ScoreloopSampleCascades::showMain()
{
	// Initialize the Scoreloop platform dependent SC_InitData_t structure to default values.
	SC_InitData_Init(&initData_);

	// Create Client instance - this is a blocking call and requires that a window was posted by the app before
	SC_Error_t rc = SC_Client_New(&client_, &initData_, GAME_ID, GAME_SECRET, GAME_VERSION, GAME_CURRENCY, GAME_LANGUAGE);

    if(rc == SC_OK) {
    	// plug Scoreloop into the BPS Event Loop
    	eventHandler_ = new ScoreloopBpsEventHandler(initData_);

        // create ScoreloopData object to expose data to QML
        scoreloopData_ = new ScoreloopData(client_);

        // start loading. signals will fire when done.
        scoreloopData_->load();
    }
    else {
        qDebug() << "Could not initialize Scoreloop: " << SC_MapErrorToStr(rc);
        return;
    }

    // create scene document from main.qml asset
    // set parent to created document to ensure it exists for the whole application lifetime
    QmlDocument *qml = QmlDocument::create("asset:///main.qml").parent(this);

    // make ScoreloopData available in qml
    qml->setContextProperty("scoreloop", scoreloopData_);

    // create root object for the UI
    AbstractPane *root = qml->createRootObject<AbstractPane>();
    // set created root object as a scene
    Application::instance()->setScene(root);
}
/*
Plugin Name
Copyright (C) 2023 sodgeIT kontakt@sodgeit.de

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include <obs-module.h>
#include <plugin-support.h>
#include <plugin-main.h>
#include <obs-frontend-api.h>
#include <QDockWidget>
#include <QMenuBar>
#include <QComboBox>
#include <thread>
#include <chrono>
#include <random>
#include <format>
#include <QLabel>
#include <iostream>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

bool obs_module_load(void)
{

	char pluginID[] = "obsadslicecontroller837611246982";
	AdControlWidget *dockWidget = new AdControlWidget();
	dockWidget->setMinimumHeight(200);
	dockWidget->setMinimumWidth(150);

	if (!obs_frontend_add_dock_by_id(pluginID, "Ad Control", dockWidget))
		throw "Could not add dock for plugin";
	obs_log(LOG_INFO, "plugin loaded successfully (version %s)",
		PLUGIN_VERSION);

	return true;
}

void obs_module_unload(void)
{
	obs_log(LOG_INFO, "plugin unloaded");
}

AdControlWidget::AdControlWidget()
{
	this->vbox->addWidget(adSelection);
	connect(adPlayButton, &QPushButton::released, this,
		&AdControlWidget::playAd);
	this->vbox->addWidget(adPlayButton);
	this->setLayout(vbox);
	reloadAds();
}

void AdControlWidget::playAd()
{

	obs_source *prevscene = obs_frontend_get_current_scene();
	obs_scene *adScene = obs_scene_create("Ad Scene");
	obs_source *scenesource = obs_scene_get_source(adScene);
	obs_source *adsource =
		obs_source_create_private("ffmpeg_source", "adSource", NULL);

	obs_frontend_set_current_scene(scenesource);

	//play ad
	//wait for ad to finish
	obs_frontend_set_current_scene(prevscene);
	/* 	obs_scene_release(adScene);
	obs_source_remove(scenesource);
	obs_source_release(adsource);
	obs_source_remove(adsource); */
}

void AdControlWidget::getAds()
{

	availableAds.clear();
	availableAds.emplace_back(AdInfo(1234, "First Option"));
	availableAds.emplace_back(AdInfo(1235, "Colgate"));
	availableAds.emplace_back(AdInfo(1236, "Food(TM)"));
	availableAds.emplace_back(AdInfo(1237, "Showgoon"));
}

void AdControlWidget::getAds(std::string APIHost)
{
	std::cout << APIHost << std::endl;
}

void AdControlWidget::reloadAds()
{
	//get menu from API
	getAds();
	adSelection->clear();
	for (AdInfo ad : availableAds) {
		adSelection->addItem(std::get<1>(ad).c_str(),
				     QVariant(std::get<0>(ad)));
	}
}
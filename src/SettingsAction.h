#pragma once

#include "actions/GroupAction.h"
#include "actions/DecimalAction.h"
#include "actions/IntegralAction.h"
#include "actions/StringAction.h"
#include "actions/TriggerAction.h"
#include "actions/DatasetPickerAction.h"
#include <PointData/DimensionPickerAction.h>
#include <PointData/PointData.h>
#include <ClusterData/ClusterData.h>
/** All GUI related classes are in the ManiVault Graphical User Interface namespace */
using namespace mv::gui;

class SettingsAction : public GroupAction
{
public:

    /**
     * Constructor
     * @param parent Pointer to parent object
     */
    SettingsAction(QObject* parent = nullptr);

public: // Action getters
	DatasetPickerAction& getAverageDatasetPickerAction() { return _averageDatasetPickerAction; } // Getter for averages point dataset picker action
	ToggleAction& getAutoUpdateAction() { return _autoUpdateAction; } // Getter for auto-update toggle action
	TriggerAction& getUpdateTriggerAction() { return _updateTriggerAction; } // Getter for update trigger action
	DatasetPickerAction& getAveragesClusterDatasetPickerAction() { return _averagesClusterDatasetPickerAction; } // Getter for averages cluster dataset picker action
	DatasetPickerAction& getPositionClusterDatasetPickerAction() { return _positionClusterDatasetPickerAction; } // Getter for position cluster dataset picker action
	DimensionPickerAction& getAveragesPointDatasetDimensionsPickerAction() { return _averagesPointDatasetDimensionsPickerAction; } // Getter for averages point dataset dimensions picker action
   

public:
	DatasetPickerAction  _averageDatasetPickerAction; // Dataset picker for averages point dataset
	ToggleAction         _autoUpdateAction; // Toggle action for auto-update
	TriggerAction        _updateTriggerAction; // Trigger action for update
	DatasetPickerAction  _averagesClusterDatasetPickerAction; // Dataset picker for averages cluster dataset
	DatasetPickerAction  _positionClusterDatasetPickerAction; // Dataset picker for position cluster dataset
	DimensionPickerAction _averagesPointDatasetDimensionsPickerAction; // Dimension picker for averages point dataset dimensions
};

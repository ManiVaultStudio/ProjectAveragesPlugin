#include "SettingsAction.h"

SettingsAction::SettingsAction(QObject* parent) :
    GroupAction(parent, "SettingsAction", true),
    _averageDatasetPickerAction(this,"Averages Point Dataset Picker Action"),
	_autoUpdateAction(this, "Auto Update Action"),
	_updateTriggerAction(this, "Update Trigger Action"),
	_averagesClusterDatasetPickerAction(this, "Averages Cluster Dataset Picker Action"),
	_positionClusterDatasetPickerAction(this, "Position Cluster Dataset Picker Action"),
	_averagesPointDatasetDimensionsPickerAction(this, "Averages Point Dataset Dimensions Picker Action")
    
{
    setText("Settings");
	
    _averageDatasetPickerAction.setFilterFunction([this](mv::Dataset<DatasetImpl> dataset) -> bool {
        return dataset->getDataType() == PointType;
        });
    
    _averagesClusterDatasetPickerAction.setFilterFunction([this](mv::Dataset<DatasetImpl> dataset) -> bool {
        return dataset->getDataType() == ClusterType;
        });
    _positionClusterDatasetPickerAction.setFilterFunction([this](mv::Dataset<DatasetImpl> dataset) -> bool {
        return dataset->getDataType() == ClusterType;
        });
    _averageDatasetPickerAction.setToolTip("Select the dataset for averages points");
	_averagesClusterDatasetPickerAction.setToolTip("Select the dataset for averages clusters");
	_positionClusterDatasetPickerAction.setToolTip("Select the dataset for position clusters");
	_autoUpdateAction.setToolTip("Enable or disable auto update of the averages points and clusters");
	_updateTriggerAction.setToolTip("Trigger an update of the averages points and clusters manually");
	_averagesPointDatasetDimensionsPickerAction.setToolTip("Select the dimensions for the averages point dataset");
    addAction(&_averageDatasetPickerAction);
	addAction(&_averagesClusterDatasetPickerAction);
    addAction(&_positionClusterDatasetPickerAction);
	addAction(&_averagesPointDatasetDimensionsPickerAction);
	addAction(&_autoUpdateAction);
    addAction(&_updateTriggerAction);
}

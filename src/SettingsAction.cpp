#include "SettingsAction.h"

using namespace mv;

SettingsAction::SettingsAction(QObject* parent) :
    GroupAction(parent, "SettingsAction", true),
    _averageDatasetPickerAction(this,"Averages Point Dataset"),
	_autoUpdateAction(this, "Auto Update Action"),
	_updateTriggerAction(this, "Update Trigger Action"),
	_averagesClusterDatasetPickerAction(this, "Averages Cluster Dataset"),
	_positionClusterDatasetPickerAction(this, "Position Cluster Dataset"),
	_averagesPointDatasetDimensionsPickerAction(this, "Averages Dataset Dimension"),
    _exportToCSVAction(this, "Export"),
    _useScRNAseqAction(this, "Compute Averages"),
    _scRNAseqDatasetPickerAction(this, "scRNAseq Dataset"),
    _scRNAseqClusterDatasetPickerAction(this, "scRNAseq Cluster Dataset"),
    _computeAveragesFromScRNAseqAction(this, "Compute Averages from scRNAseq")
    
{
    setText("Settings");
    setSerializationName("ProjectAveragesPlugin:Settings");
	_averageDatasetPickerAction.setSerializationName("ProjectAveragesPlugin:AverageDatasetPickerAction");
	_autoUpdateAction.setSerializationName("ProjectAveragesPlugin:AutoUpdateAction");
	_averagesClusterDatasetPickerAction.setSerializationName("ProjectAveragesPlugin:AveragesClusterDatasetPickerAction");
	_positionClusterDatasetPickerAction.setSerializationName("ProjectAveragesPlugin:PositionClusterDatasetPickerAction");
	_averagesPointDatasetDimensionsPickerAction.setSerializationName("ProjectAveragesPlugin:AveragesPointDatasetDimensionsPickerAction");

    _useScRNAseqAction.setSerializationName("ProjectAveragesPlugin:UseScRNAseqAction");
    _scRNAseqDatasetPickerAction.setSerializationName("ProjectAveragesPlugin:ScRNAseqDatasetPickerAction");
    _scRNAseqClusterDatasetPickerAction.setSerializationName("ProjectAveragesPlugin:ScRNAseqClusterDatasetPickerAction");

	
    _averageDatasetPickerAction.setFilterFunction([this](mv::Dataset<DatasetImpl> dataset) -> bool {
        return dataset->getDataType() == PointType;
        });
    
    _averagesClusterDatasetPickerAction.setFilterFunction([this](mv::Dataset<DatasetImpl> dataset) -> bool {
        return dataset->getDataType() == ClusterType;
        });

    _positionClusterDatasetPickerAction.setFilterFunction([this](mv::Dataset<DatasetImpl> dataset) -> bool {
        return dataset->getDataType() == ClusterType;
        });

    _scRNAseqDatasetPickerAction.setFilterFunction([this](mv::Dataset<DatasetImpl> dataset) -> bool {
        return dataset->getDataType() == PointType;
        });

    _scRNAseqClusterDatasetPickerAction.setFilterFunction([this](mv::Dataset<DatasetImpl> dataset) -> bool {
        return dataset->getDataType() == ClusterType;
        });

    _averageDatasetPickerAction.setToolTip("Select the dataset for averages points");
	_averagesClusterDatasetPickerAction.setToolTip("Select the dataset for averages clusters");
	_positionClusterDatasetPickerAction.setToolTip("Select the dataset for position clusters");
	_autoUpdateAction.setToolTip("Enable or disable auto update of the averages points and clusters");
	_updateTriggerAction.setToolTip("Trigger an update of the averages points and clusters manually");
	_averagesPointDatasetDimensionsPickerAction.setToolTip("Select the dimensions for the averages point dataset");
    _exportToCSVAction.setToolTip("Export the mapped values to csv");

    _useScRNAseqAction.setToolTip("Enable or disable the computation of averages from scRNAseq data");
    _scRNAseqDatasetPickerAction.setToolTip("Select the dataset for scRNAseq data");
    _scRNAseqClusterDatasetPickerAction.setToolTip("Select the dataset for scRNAseq clusters");
    _computeAveragesFromScRNAseqAction.setToolTip("Compute the averages from the scRNAseq data");

    // set the visibility of the scRNAseq related actions invisible
    _useScRNAseqAction.setChecked(false);
    _scRNAseqDatasetPickerAction.setVisible(false);
    _scRNAseqClusterDatasetPickerAction.setVisible(false);
    _computeAveragesFromScRNAseqAction.setVisible(false);

    connect(&_useScRNAseqAction, &ToggleAction::toggled, this, [this](bool checked) {
        _scRNAseqDatasetPickerAction.setVisible(checked);
        _scRNAseqClusterDatasetPickerAction.setVisible(checked);
        _computeAveragesFromScRNAseqAction.setVisible(checked);
        });

    addAction(&_averageDatasetPickerAction);
	addAction(&_averagesClusterDatasetPickerAction);
    addAction(&_positionClusterDatasetPickerAction);
	addAction(&_averagesPointDatasetDimensionsPickerAction);
	addAction(&_autoUpdateAction);
    addAction(&_updateTriggerAction);
    addAction(&_exportToCSVAction);

    addAction(&_useScRNAseqAction);
    addAction(&_scRNAseqDatasetPickerAction);
    addAction(&_scRNAseqClusterDatasetPickerAction);
    addAction(&_computeAveragesFromScRNAseqAction);
}


void SettingsAction::fromVariantMap(const QVariantMap& variantMap)
{
    WidgetAction::fromVariantMap(variantMap);
    _averageDatasetPickerAction.fromParentVariantMap(variantMap);
    _averagesClusterDatasetPickerAction.fromParentVariantMap(variantMap);
    _positionClusterDatasetPickerAction.fromParentVariantMap(variantMap);
    _averagesPointDatasetDimensionsPickerAction.fromParentVariantMap(variantMap);
    _autoUpdateAction.fromParentVariantMap(variantMap);

    //_useScRNAseqAction.fromParentVariantMap(variantMap);
    _scRNAseqDatasetPickerAction.fromParentVariantMap(variantMap);
    _scRNAseqClusterDatasetPickerAction.fromParentVariantMap(variantMap);
}

QVariantMap SettingsAction::toVariantMap() const
{
    QVariantMap variantMap = WidgetAction::toVariantMap();
    _averageDatasetPickerAction.insertIntoVariantMap(variantMap);
    _averagesClusterDatasetPickerAction.insertIntoVariantMap(variantMap);
    _positionClusterDatasetPickerAction.insertIntoVariantMap(variantMap);
    _averagesPointDatasetDimensionsPickerAction.insertIntoVariantMap(variantMap);
    _autoUpdateAction.insertIntoVariantMap(variantMap);

    //_useScRNAseqAction.insertIntoVariantMap(variantMap);// No need to serialize
    _scRNAseqDatasetPickerAction.insertIntoVariantMap(variantMap);
    _scRNAseqClusterDatasetPickerAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
#include "ProjectAveragesPlugin.h"

#include <event/Event.h>

#include <QtCore>
#include <QDebug>

Q_PLUGIN_METADATA(IID "studio.manivault.ProjectAveragesPlugin")

using namespace mv;
using namespace mv::plugin;



ProjectAveragesPlugin::ProjectAveragesPlugin(const PluginFactory* factory) :
    AnalysisPlugin(factory)
{
}

void ProjectAveragesPlugin::init()
{
    // get the input dataset
    _positionDataset = getInputDataset<Points>(); // this should be plotted as a 2D scatter plot 

    // initialize an output dataset
    if (!outputDataInit())
    {
        setOutputDataset(mv::data().createDerivedDataset("Mapped dataset", getInputDataset(), getInputDataset()));
    }
    

	/*auto positionDatasetChildren = _positionDataset->getChildren();
    Datasets validChildren;
    for (const auto& child : positionDatasetChildren)
    {
        if (child->getDataType() == ClusterType)
        {
            validChildren.append(child);
        }
	}
	_settingsAction.getPositionClusterDatasetPickerAction().setDatasets(validChildren);*/

    const auto triggerUpdate = [this]() {triggerMapping(); };
    connect(&_settingsAction.getUpdateTriggerAction(), &TriggerAction::triggered, this, triggerUpdate);


    const auto updateAverageDatasetPickerAction = [this]() {
        
        if (_settingsAction.getAverageDatasetPickerAction().getCurrentDataset().isValid())
        {
			_settingsAction.getAveragesPointDatasetDimensionsPickerAction().setPointsDataset(_settingsAction.getAverageDatasetPickerAction().getCurrentDataset());
        }
        bool validity = checkValidity();
        
        
        if (validity)
        {
            if (_settingsAction.getAutoUpdateAction().isChecked())
            {
                triggerMapping();
            }
            
        }

        };
    connect(&_settingsAction.getAverageDatasetPickerAction(), &DatasetPickerAction::currentIndexChanged, this, updateAverageDatasetPickerAction);

    const auto updateAveragesClusterDatasetPickerAction = [this]() {
        bool validity = checkValidity();
        if (validity)
        {
            if (_settingsAction.getAutoUpdateAction().isChecked())
            {
                triggerMapping();
            }

        }
        };
    connect(&_settingsAction.getAveragesClusterDatasetPickerAction(), &DatasetPickerAction::currentIndexChanged, this, updateAveragesClusterDatasetPickerAction);

    const auto updatePositionClusterDatasetPickerAction = [this]() {
        bool validity = checkValidity();
        if (validity)
        {
            if (_settingsAction.getAutoUpdateAction().isChecked())
            {
                triggerMapping();
            }

        }
        };
    connect(&_settingsAction.getPositionClusterDatasetPickerAction(), &DatasetPickerAction::currentIndexChanged, this, updatePositionClusterDatasetPickerAction);

    const auto updateAveragesPointDatasetDimensionsPickerAction = [this]() {
        bool validity = checkValidity();
        if (validity)
        {
            if (_settingsAction.getAutoUpdateAction().isChecked())
            {
                triggerMapping();
            }

        }
        };
    connect(&_settingsAction.getAveragesPointDatasetDimensionsPickerAction(), &DimensionPickerAction::currentDimensionIndexChanged, this, updateAveragesPointDatasetDimensionsPickerAction);

    bool validity=checkValidity();

    getOutputDataset()->addAction(_settingsAction);
    
}

bool ProjectAveragesPlugin::checkValidity()
{
    if (!_positionDataset.isValid() || !_settingsAction.getAverageDatasetPickerAction().getCurrentDataset().isValid() || !_settingsAction.getAveragesClusterDatasetPickerAction().getCurrentDataset().isValid() || !_settingsAction.getPositionClusterDatasetPickerAction().getCurrentDataset().isValid() || _settingsAction.getAveragesPointDatasetDimensionsPickerAction().getCurrentDimensionIndex() < 0)
    {
        _settingsAction.getUpdateTriggerAction().setDisabled(true);
		return false;
    }
    else
    {
        _settingsAction.getUpdateTriggerAction().setEnabled(true);
		return true;
	}
}

void ProjectAveragesPlugin::triggerMapping()
{
    if (!_positionDataset.isValid() || !_settingsAction.getAverageDatasetPickerAction().getCurrentDataset().isValid() || !_settingsAction.getAveragesClusterDatasetPickerAction().getCurrentDataset().isValid() || !_settingsAction.getPositionClusterDatasetPickerAction().getCurrentDataset().isValid() || _settingsAction.getAveragesPointDatasetDimensionsPickerAction().getCurrentDimensionIndex()<0)
    {
        qDebug() << "Position dataset or average dataset is not set or invalid";
		return;
    }
    else
    {
        mapAveragesToScalars();
    }
}

void ProjectAveragesPlugin::mapAveragesToScalars()
{

    Dataset<Points> averageDataset = _settingsAction.getAverageDatasetPickerAction().getCurrentDataset();
    if (!averageDataset.isValid())
    {
                qDebug() << "Average dataset is not set or invalid";
				return;
    }
    Dataset<Clusters> labelDatasetForAverages = _settingsAction.getAveragesClusterDatasetPickerAction().getCurrentDataset();
    if (!labelDatasetForAverages.isValid())
    {
        qDebug() << "Label dataset for averages is not set or invalid";
        return;
    }

    Dataset<Clusters> labelDataset = _settingsAction.getPositionClusterDatasetPickerAction().getCurrentDataset();
    if (!labelDataset.isValid())
    {
        qDebug() << "Label dataset for positions is not set or invalid";
        return;
    }
    int averageDatasetSelectedDimension = _settingsAction.getAveragesPointDatasetDimensionsPickerAction().getCurrentDimensionIndex();
    if (averageDatasetSelectedDimension < 0 || averageDatasetSelectedDimension >= averageDataset->getNumDimensions())
    {
        qDebug() << "Selected dimension index is out of bounds for the average dataset";
        return;
	}


    auto& datasetTask = getOutputDataset()->getTask();


    datasetTask.setName("Mapping averages");


    datasetTask.setRunning();


    datasetTask.setProgress(0.0f);



    // store the labels of average dataset in  a vector
    _labelsInAverages.resize(averageDataset->getNumPoints());


    const QVector<Cluster>& labelClustersInAverages = labelDatasetForAverages->getClusters();
    for (int i = 0; i < labelClustersInAverages.size(); ++i)
    {
        const auto& cluster = labelClustersInAverages[i];
        const auto ptIndices = cluster.getIndices();
        for (int ptIndex : ptIndices)
        {
            _labelsInAverages[ptIndex] = cluster.getName();
        }
    }
    _mappedScalars.resize(_positionDataset->getNumPoints(), 0.0f);

    std::vector<float> averagesForSelectedDimension;
    averageDataset->extractDataForDimension(averagesForSelectedDimension, averageDatasetSelectedDimension); 
    qDebug() << "test dim " << averageDataset->getDimensionNames()[averageDatasetSelectedDimension];

    const int numPoints = _positionDataset->getNumPoints();

    // Iterate over the points in the position dataset
    const QVector<Cluster>& labelClusters = labelDataset->getClusters();

    for (int i = 0; i < averagesForSelectedDimension.size(); ++i) {

        QString clusterNameInAverage = _labelsInAverages[i];
        //qDebug() << "clusterNameInAverage: " << clusterNameInAverage;

        // hard-coded to remove "cluster_" prefix in clusterName TODO: generalize this
        /*if (clusterNameInAverage.startsWith("cluster_")) {
            clusterNameInAverage = clusterNameInAverage.mid(8);
        }*/

        bool found = false;

        //search for the cluster name in labelClusters
        for (const auto& cluster : labelClusters) {

            QString clusterNameInEmbedding = cluster.getName();

            //qDebug() << "clusterNameInEmbedding: " << clusterNameInEmbedding;
            //qDebug() << "Comparing: " << clusterNameInAverage << " vs " << clusterNameInEmbedding;
            if (clusterNameInAverage == clusterNameInEmbedding) 
            {
                const auto& ptIndices = cluster.getIndices();
                for (int j = 0; j < ptIndices.size(); ++j) {
                    int ptIndex = ptIndices[j];
                    if (ptIndex >= 0 && ptIndex < _mappedScalars.size()) {
                        _mappedScalars[ptIndex] = averagesForSelectedDimension[i];
                    }
                }
                found = true;
                break;
            }
        }
    }


    // Update the output dataset with the mapped scalars
    getOutputDataset<Points>()->setData<float>(_mappedScalars.data(), _mappedScalars.size(), 1);
    events().notifyDatasetDataChanged(getOutputDataset<Points>());
    QString geneName = _settingsAction.getAveragesPointDatasetDimensionsPickerAction().getCurrentDimensionName();
    if (geneName.isEmpty())
    {
        getOutputDataset<Points>()->setDimensionNames({ geneName });
        events().notifyDatasetDataDimensionsChanged(getOutputDataset<Points>());
    }
    datasetTask.setProgressDescription("Finalizing");
    datasetTask.setProgress(100.0f);
    datasetTask.setFinished();
}


void ProjectAveragesPlugin::onDataEvent(mv::DatasetEvent* dataEvent)
{
    // The data event has a type so that we know what type of data event occurred (e.g. data added, changed, removed, renamed, selection changes)
    switch (dataEvent->getType()) {

        // A points dataset was added
        case EventType::DatasetAdded:
        {
            // Cast the data event to a data added event
            const auto dataAddedEvent = static_cast<DatasetAddedEvent*>(dataEvent);

            // Get the GUI name of the added points dataset and print to the console
            qDebug() << dataAddedEvent->getDataset()->getGuiName() << "was added";

            break;
        }

        // Points dataset data has changed
        case EventType::DatasetDataChanged:
        {
            // Cast the data event to a data changed event
            const auto dataChangedEvent = static_cast<DatasetDataChangedEvent*>(dataEvent);

            // Get the GUI name of the points dataset of which the data changed and print to the console
            qDebug() << dataChangedEvent->getDataset()->getGuiName() << "data changed";

            break;
        }

        // Points dataset data was removed
        case EventType::DatasetRemoved:
        {
            // Cast the data event to a data removed event
            const auto dataRemovedEvent = static_cast<DatasetRemovedEvent*>(dataEvent);

            // Get the GUI name of the removed points dataset and print to the console
            qDebug() << dataRemovedEvent->getDataset()->getGuiName() << "was removed";

            break;
        }

        // Points dataset selection has changed
        case EventType::DatasetDataSelectionChanged:
        {
            // Cast the data event to a data selection changed event
            const auto dataSelectionChangedEvent = static_cast<DatasetDataSelectionChangedEvent*>(dataEvent);

            // Get points dataset
            const auto& changedDataSet = dataSelectionChangedEvent->getDataset();

            // Get the selection set that changed
            const auto selectionSet = changedDataSet->getSelection<Points>();

            // Print to the console
            qDebug() << changedDataSet->getGuiName() << "selection has changed";

            break;
        }

        default:
            break;
    }
}

// =============================================================================
// Serialization
// =============================================================================

void ProjectAveragesPlugin::fromVariantMap(const QVariantMap& variantMap)
{
    AnalysisPlugin::fromVariantMap(variantMap);
    mv::util::variantMapMustContain(variantMap, "ProjectAveragesPlugin:Settings");
    _settingsAction.fromVariantMap(variantMap["ProjectAveragesPlugin:Settings"].toMap());
}

QVariantMap ProjectAveragesPlugin::toVariantMap() const
{
    QVariantMap variantMap = AnalysisPlugin::toVariantMap();

    _settingsAction.insertIntoVariantMap(variantMap);

    return variantMap;
}


// =============================================================================
// Plugin Factory 
// =============================================================================

ProjectAveragesPluginFactory::ProjectAveragesPluginFactory()
{
    
}

AnalysisPlugin* ProjectAveragesPluginFactory::produce()
{
    return new ProjectAveragesPlugin(this);
}

mv::DataTypes ProjectAveragesPluginFactory::supportedDataTypes() const
{
    DataTypes supportedTypes;

    // This analysis plugin is compatible with points datasets
    supportedTypes.append(PointType);

    return supportedTypes;
}

mv::gui::PluginTriggerActions ProjectAveragesPluginFactory::getPluginTriggerActions(const mv::Datasets& datasets) const
{
    PluginTriggerActions pluginTriggerActions;

    const auto getPluginInstance = [this](const Dataset<Points>& dataset) -> ProjectAveragesPlugin* {
        return dynamic_cast<ProjectAveragesPlugin*>(plugins().requestPlugin(getKind(), { dataset }));
    };

    const auto numberOfDatasets = datasets.count();

    if (numberOfDatasets >= 1 && PluginFactory::areAllDatasetsOfTheSameType(datasets, PointType)) {
        auto pluginTriggerAction = new PluginTriggerAction(const_cast<ProjectAveragesPluginFactory*>(this), this, "Project Averages", "Project cluster averages to a 2D map", icon(), [this, getPluginInstance, datasets](PluginTriggerAction& pluginTriggerAction) -> void {
            for (auto dataset : datasets)
                getPluginInstance(dataset);
            });

        pluginTriggerActions << pluginTriggerAction;
    }

    return pluginTriggerActions;
}

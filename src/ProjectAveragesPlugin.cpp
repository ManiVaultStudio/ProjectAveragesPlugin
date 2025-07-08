#include "ProjectAveragesPlugin.h"

#include "PointData/PointData.h"
#include "ClusterData/ClusterData.h"

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
    setOutputDataset(mv::data().createDerivedDataset("Mapped dataset", getInputDataset(), getInputDataset()));

    // get the averages dataset that is used for mapping the averages to the points dataset
    for (const auto& data : mv::data().getAllDatasets())
    {
        //qDebug() << data->getGuiName();
        if (data->getGuiName() == "marm_Cluster_v4_metacell") {// TODO: generize this to work with any selected data
            qDebug() << "Found average dataset: " << data->getGuiName();
            _averageDataset = data;
            break;      
        }
    }

    // store the labels of average dataset in  a vector
    _labelsInAverages.resize(_averageDataset->getNumPoints());
    Dataset<Clusters> labelDatasetForAverages;
    for (const auto& data : mv::data().getAllDatasets()) // TODO: generize this to work with any selected data
    {
        if (data->getGuiName() == "cell_type") { 
            labelDatasetForAverages = data;
            qDebug() << "Found label dataset fo raverages: " << labelDatasetForAverages->getGuiName();
            break;
        }
    }
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


    // load the cluster data of the position dataset
    for (const auto& data : mv::data().getAllDatasets()) // TODO: generize this to work with any selected data
    {
        if (data->getGuiName() == "CDM_Cluster_label") {
            _labelDataset = data;
            qDebug() << "Found label dataset: " << _labelDataset->getGuiName();
            break;
        }
    }

    mapAveragesToScalars();
    
}

void ProjectAveragesPlugin::mapAveragesToScalars()
{
    // Check if the average dataset is valid
    if (!_averageDataset.isValid()) 
    {
        qDebug() << "Average dataset is not set";
        return;
    }

    _mappedScalars.resize(_positionDataset->getNumPoints(), 0.0f);

    std::vector<float> averagesForSelectedDimension;
    _averageDataset->extractDataForDimension(averagesForSelectedDimension, 777); // TODO: here dimension index should be defined by user
    qDebug() << "test dim " << _averageDataset->getDimensionNames()[777];

    const int numPoints = _positionDataset->getNumPoints();

    // Iterate over the points in the position dataset
    const QVector<Cluster>& labelClusters = _labelDataset->getClusters();

    for (int i = 0; i < averagesForSelectedDimension.size(); ++i) {

        QString clusterNameInAverage = _labelsInAverages[i];
        qDebug() << "clusterNameInAverage: " << clusterNameInAverage;

        // hard-coded to remove "cluster_" prefix in clusterName
        if (clusterNameInAverage.startsWith("cluster_")) {
            clusterNameInAverage = clusterNameInAverage.mid(8);
        }
        
        bool found = false;

        //search for the cluster name in labelClusters
        for (const auto& cluster : labelClusters) {

            QString clusterNameInEmbedding = cluster.getName();

            //qDebug() << "clusterNameInEmbedding: " << clusterNameInEmbedding;

            if (clusterNameInAverage == clusterNameInEmbedding) {
                const auto& ptIndices = cluster.getIndices();
                for (int j = 0; j < ptIndices.size(); ++j) {
                    int ptIndex = ptIndices[j];
                    _mappedScalars[ptIndex] = averagesForSelectedDimension[i];
                }
                found = true;
                break;
            }
        }
    }

  
    // Update the output dataset with the mapped scalars
    getOutputDataset<Points>()->setData<float>(_mappedScalars.data(), _mappedScalars.size(), 1);
    events().notifyDatasetDataChanged(getOutputDataset<Points>());
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
}

QVariantMap ProjectAveragesPlugin::toVariantMap() const
{
    QVariantMap variantMap = AnalysisPlugin::toVariantMap();

    //_settingsAction.insertIntoVariantMap(variantMap);

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

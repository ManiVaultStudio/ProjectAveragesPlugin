#include "ProjectAveragesPlugin.h"

#include <event/Event.h>
#include <util/Serialization.h>
#include <PointData/InfoAction.h>
#include <PointData/DimensionsPickerAction.h>

#include <QtCore>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

Q_PLUGIN_METADATA(IID "studio.manivault.ProjectAveragesPlugin")using namespace mv;
using namespace mv::plugin;



ProjectAveragesPlugin::ProjectAveragesPlugin(const PluginFactory* factory) :
    AnalysisPlugin(factory),
    _settingsAction(this)
{
}

void ProjectAveragesPlugin::init()
{
    // get the input dataset
    _positionDataset = getInputDataset<Points>(); // this should be plotted as a 2D scatter plot 

    // initialize an output dataset
    if (!outputDataInit())
    {
        setOutputDataset(Dataset<Points>(mv::data().createDerivedDataset("Mapped dataset", getInputDataset(), getInputDataset())));
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

    connect(&_settingsAction.getExportToCSVAction(), &TriggerAction::triggered, this, [this]() {
        exportMappedScalarsToCSV();
    });

    connect(&_settingsAction.getComputeAveragesFromScRNAseqAction(), &TriggerAction::triggered, this, [this]() {
        computeAveragesFromScRNAseq();
        });

    bool validity=checkValidity();

    getOutputDataset()->addAction(_settingsAction);

    // Automatically focus on the settings action
    getOutputDataset<Points>()->_infoAction->collapse();
    
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
            if (ptIndex >= 0 && ptIndex < static_cast<int>(_labelsInAverages.size())) {
                _labelsInAverages[ptIndex] = cluster.getName();
            }
            else {

                qCritical() << "ptIndex out of bounds:" << ptIndex << "for cluster" << cluster.getName() << "Check cluster data set and try again";
                return;
            }
        }
    }
    _mappedScalars.resize(_positionDataset->getNumPoints(), 0.0f);

    std::vector<float> averagesForSelectedDimension;
    averageDataset->extractDataForDimension(averagesForSelectedDimension, averageDatasetSelectedDimension);
    //qDebug() << "test dim " << averageDataset->getDimensionNames()[averageDatasetSelectedDimension];

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
    QString geneName = _settingsAction.getAveragesPointDatasetDimensionsPickerAction().getCurrentDimensionName();
    if (geneName.isEmpty())
    {
        geneName = "Dim" + QString::number(averageDatasetSelectedDimension);
    }

    // Update the output dataset with the mapped scalars
    getOutputDataset<Points>()->setData<float>(_mappedScalars.data(), _mappedScalars.size(), 1);
    events().notifyDatasetDataChanged(getOutputDataset<Points>());

    getOutputDataset<Points>()->setDimensionNames({ geneName });
	events().notifyDatasetDataDimensionsChanged(getOutputDataset<Points>());
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

void ProjectAveragesPlugin::exportMappedScalarsToCSV()
{
    //qDebug() << "exportMappedScalarsToCSV()";

    if (_mappedScalars.size() == 0)
    {
        QMessageBox::warning(nullptr, "Warning", "No values imputed, impute a feature first.");
        return;
    }

    QString geneName = _settingsAction.getAveragesPointDatasetDimensionsPickerAction().getCurrentDimensionName();
    //qDebug() << "Export " << geneName;

    // get cell labels
    QVariantList parentSampleNameList;
    if (_positionDataset.isValid() && _positionDataset->hasProperty("Sample Names"))
    {
        //qDebug() << "PositionDataset->getGuiName() " << _positionDataset->getGuiName();
        parentSampleNameList = _positionDataset->getProperty("Sample Names").toList();
    }

    QString safeFileName = geneName;
    safeFileName.replace(":", "_"); // Avoid :, changes "chr4:135..." to "chr4_135..."

    QString defaultPathAndName = safeFileName + ".csv";

    // save dialog
    QString fullPath = QFileDialog::getSaveFileName(
        nullptr,
        QObject::tr("Save Mapped Scalars as CSV"), 
        defaultPathAndName,  
        QObject::tr("CSV Files (*.csv);;All Files (*)") // Filter so it saves as .csv
    );

    if (fullPath.isEmpty()) {
        qDebug() << "Saving canceled." << geneName;
        return; 
    }

    QFile file(fullPath);

    // Check if the file successfully opens
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);

        // Write CSV headers
        out << "cell_label," << geneName << "\n";

        for (int i = 0; i < _mappedScalars.size(); ++i)
        {
            QString cellLabel = "Unknown"; // in case no cellLabel

            // Safety check 
            if (i < parentSampleNameList.size()) {
                cellLabel = parentSampleNameList[i].toString();
            }

            out << cellLabel << "," << _mappedScalars[i] << "\n";
        }

        file.close();

        qDebug() << "Successfully saved to:" << fullPath;
    }
    else
    {
        qDebug() << "Failed to open file for writing:" << fullPath;
    }

}

void ProjectAveragesPlugin::computeAveragesFromScRNAseq()
{
    // compute averages from scRNAseq data using the selected cluster dataset
    // store in a new points dataset and generate a cluster dataset, add them data hierarchy
    // set the computed dataset as the average dataset in the settings action
    Dataset<Points> scDataset = _settingsAction.getScRNAseqDatasetPickerAction().getCurrentDataset();
    Dataset<Clusters> scClusterDataset = _settingsAction.getScRNAseqClusterDatasetPickerAction().getCurrentDataset();

    if (!scDataset.isValid() || !scClusterDataset.isValid())
    {
        qDebug() << "ScRNAseq dataset or cluster dataset is not set or invalid";
        return;
    }

    const QVector<Cluster>& sourceClusters = scClusterDataset->getClusters();
    const int numClusters = sourceClusters.size();
    const int numGenes = scDataset->getNumDimensions();

    if (numClusters == 0 || numGenes == 0)
    {
        qDebug() << "No clusters or no dimensions in scRNAseq dataset";
        return;
    }

    qDebug() << "Computing averages from scRNAseq dataset with" << numClusters << "clusters and" << numGenes << "dimensions...";

    // Prepare storage for averaged data: rows = clusters, cols = genes
    std::vector<float> averages;
    averages.resize(static_cast<std::size_t>(numClusters) * static_cast<std::size_t>(numGenes), 0.0f);

    // For each gene (dimension) extract full column once and compute per-cluster averages.
    for (std::uint64_t geneIdx = 0; geneIdx < numGenes; ++geneIdx)
    {
        std::vector<float> geneValues;
        scDataset->extractDataForDimension(geneValues, geneIdx);

        if (geneValues.empty())
            continue;

        for (std::uint64_t clusterIdx = 0; clusterIdx < numClusters; ++clusterIdx)
        {
            const auto& indices = sourceClusters[clusterIdx].getIndices();
            double sum = 0.0;
            std::size_t count = 0;

            for (auto idx : indices)
            {
                if (idx >= 0 && idx < static_cast<int>(geneValues.size()))
                {
                    sum += static_cast<double>(geneValues[static_cast<std::size_t>(idx)]);
                    ++count;
                }
                else
                {
                    qCritical() << "Index out of bounds while averaging scRNAseq:" << idx;
                }
            }

            const float avg = (count > 0) ? static_cast<float>(sum / static_cast<double>(count)) : 0.0f;
            averages[static_cast<std::size_t>(clusterIdx) * static_cast<std::size_t>(numGenes) + static_cast<std::size_t>(geneIdx)] = avg;
        }
    }

    // Create a points dataset to store cluster-averages (rows = clusters, cols = genes)
    Dataset<Points> averagesDataset = Dataset<Points>(mv::data().createDataset("Points", "Averages from scRNAseq"));
    if (!averagesDataset.isValid())
    {
        qCritical() << "Failed to create averages dataset";
        return;
    }

    averagesDataset->setData<float>(averages.data(), static_cast<std::size_t>(numClusters), static_cast<std::size_t>(numGenes));

    // Copy gene/dimension names if available, otherwise create generic names
    std::vector<QString> geneNames;
    if (scDataset->getDimensionNames().size() == static_cast<std::size_t>(numGenes))
        geneNames = scDataset->getDimensionNames();
    else
    {
        geneNames.resize(numGenes);
        for (std::uint64_t i = 0; i < numGenes; ++i)
            geneNames[i] = QString("Dim%1").arg(i);
    }

    averagesDataset->setDimensionNames(geneNames);
    events().notifyDatasetDataChanged(averagesDataset);
    events().notifyDatasetDataDimensionsChanged(averagesDataset);

    // Create a clusters dataset describing the rows of the averages dataset.
    // Each cluster corresponds to one averaged row -> cluster indices point to the single row index in the averages dataset.
    Dataset<Clusters> averagesClusterDataset = Dataset<Clusters>(mv::data().createDataset("Cluster", "Averages clusters", averagesDataset));
    if (!averagesClusterDataset.isValid())
    {
        qWarning() << "Failed to create averages cluster dataset. Averages dataset created without cluster metadata.";
    }
    else
    {
        // Clear existing clusters then add a cluster per averaged row using original cluster names
        averagesClusterDataset->getClusters().clear();
        for (std::uint64_t clusterIdx = 0; clusterIdx < numClusters; ++clusterIdx)
        {
            Cluster newCluster;
            newCluster.setName(sourceClusters[clusterIdx].getName());

            std::vector<std::uint32_t> idxVec;
            idxVec.push_back(static_cast<std::uint32_t>(clusterIdx));
            newCluster.setIndices(idxVec);

            averagesClusterDataset->addCluster(newCluster);
        }

        events().notifyDatasetDataChanged(averagesClusterDataset);
    }

    // Try to set the newly created averages dataset as the selected average dataset in the settings action.
    // If not available, select the dataset from the GUI manually.
    try
    {
        _settingsAction.getAverageDatasetPickerAction().setCurrentDataset(averagesDataset);
        _settingsAction.getAveragesClusterDatasetPickerAction().setCurrentDataset(averagesClusterDataset);
    }
    catch (...)
    {
        qDebug() << "Could not set the newly created averages dataset as the selected average dataset. Please select it manually.";
    }
    
    qDebug() << "Computed averages from scRNAseq: created dataset with" << numClusters << "rows and" << numGenes << "dimensions.";
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

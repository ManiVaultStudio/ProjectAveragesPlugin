#pragma once

#include <AnalysisPlugin.h>

#include "SettingsAction.h"

#include "graphics/Vector2f.h"

#include <Dataset.h>
#include <PointData/PointData.h>
#include <ClusterData/ClusterData.h>
#include <QRegularExpression>
#include <QRandomGenerator>
#include <QtMath>

#include <unordered_map>

/** All plugin related classes are in the ManiVault plugin namespace */
using namespace mv::plugin;

/** Vector classes used in this plugin are in the ManiVault namespace */
using namespace mv;

class ProjectAveragesPlugin : public AnalysisPlugin
{
Q_OBJECT


public:

    /**
     * Constructor
     * @param factory Pointer to the plugin factory
     */
    ProjectAveragesPlugin(const PluginFactory* factory);

    /** Destructor */
    ~ProjectAveragesPlugin() override = default;

    /**
     * This function is called by the core after the analysis plugin has been created
     *
     * Typical implementations of this function focus on the generation of output data
     * and responding to events which are sent by the core.
    */
    void init() override;

    void mapAveragesToScalars();
    bool checkValidity();
    void triggerMapping();

    void precomputeForSpatial(); // precompute things needed for spatial
    void precomputeForAverages(); // precompute things needed for averaging
    /**
     * Invoked when a points data event occurs
     * @param dataEvent Data event which occurred
     */
    void onDataEvent(mv::DatasetEvent* dataEvent);

public: // Serialization
    /**
    * Load plugin from variant map
    * @param Variant map representation of the plugin
    */
    Q_INVOKABLE void fromVariantMap(const QVariantMap& variantMap) override;

    /**
    * Save plugin to variant map
    * @return Variant map representation of the plugin
    */
    Q_INVOKABLE QVariantMap toVariantMap() const override;

private:
    SettingsAction      _settingsAction;    /** The place where settings are stored (more info in SettingsAction.h) */
    std::vector<float>  _mappedScalars; 
    Dataset<Points> _positionDataset;

    std::unordered_map<QString, int> _clusterAliasToRowMap;// map cluster name to the row index in average dataset
    std::vector<QString> _clusterLabelsForEachSpatialCell; // same order as the spatial dataset
};

/**
 * Project averages plugin factory class
 */
class ProjectAveragesPluginFactory : public AnalysisPluginFactory
{
    Q_INTERFACES(mv::plugin::AnalysisPluginFactory mv::plugin::PluginFactory)
    Q_OBJECT
    Q_PLUGIN_METADATA(IID   "studio.manivault.ProjectAveragesPlugin"
                      FILE  "PluginInfo.json")

public:

    /** Default constructor */
    ProjectAveragesPluginFactory();

    /** Creates an instance of the example analysis plugin */
    AnalysisPlugin* produce() override;

    /** Returns the data types that are supported by the example analysis plugin */
    mv::DataTypes supportedDataTypes() const override;

    /** Enable right-click on data set to open analysis */
    PluginTriggerActions getPluginTriggerActions(const mv::Datasets& datasets) const override;
};

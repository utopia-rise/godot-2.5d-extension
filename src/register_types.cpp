#include "register_types.h"

#include "isometric_server.h"
#include "isometric_string_names.h"
#include "node/isometric_map.h"
#include "node/isometric_positionable.h"
#include "resource/isometric_configuration.h"
#include "resource/positionable_set.h"

#include <classes/engine.hpp>
#include <core/class_db.hpp>

#ifdef TOOLS_ENABLED
#include "editor/commands/add_layer_command.h"
#include "editor/commands/add_positionable_command.h"
#include "editor/commands/composite_command.h"
#include "editor/commands/move_editor_plane_command.h"
#include "editor/commands/revert_command.h"
#include "editor/commands/rotate_editor_plane_command.h"
#include "editor/commands/select_positionable_command.h"
#include "editor/commands/set_layer_visibility_command.h"
#include "editor/commands/set_positionable_visibility_command.h"
#include "editor/editor_utils.h"
#include "editor/inspector/layers_editor.h"
#include "editor/inspector/positionable_selection_pane.h"
#include "editor/inspector/positionable_set_editor.h"
#include "editor/isometric_editor_plugin.h"
#include "editor/isometric_tile_editor_plugin.h"
#include "editor/positionable_scenes_cache_manager.h"
#include "editor/positionable_selector_manager.h"
#include "editor/positionable_set_editor_plugin.h"

#include <classes/editor_plugin_registration.hpp>
#endif

using namespace godot;

static constexpr const char* SERVER_SINGLETON_NAME = "IsometricServer";

void initialize_isometric_maps_module(ModuleInitializationLevel p_level) {
    if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
        IsometricStringNames::create();

        GDREGISTER_CLASS(IsometricServer);
        IsometricServer::create_server();
        Engine::get_singleton()->register_singleton(SERVER_SINGLETON_NAME, IsometricServer::get_instance());

        GDREGISTER_CLASS(resource::IsometricConfiguration);
        GDREGISTER_CLASS(resource::PositionableSet);
        GDREGISTER_CLASS(node::IsometricPositionable);
        GDREGISTER_CLASS(node::IsometricMap);
    }

#ifdef TOOLS_ENABLED
    if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
        editor::PositionableSelectorManager::create();
        editor::PositionableScenesCacheManager::create();

        GDREGISTER_ABSTRACT_CLASS(editor::commands::Command);
        GDREGISTER_INTERNAL_CLASS(editor::commands::AddLayerCommand);
        GDREGISTER_INTERNAL_CLASS(editor::commands::AddPositionableCommand);
        GDREGISTER_INTERNAL_CLASS(editor::commands::CompositeCommand);
        GDREGISTER_INTERNAL_CLASS(editor::commands::MoveEditorPlaneCommand);
        GDREGISTER_INTERNAL_CLASS(editor::commands::RevertCommand);
        GDREGISTER_INTERNAL_CLASS(editor::commands::RotateEditorPlaneCommand);
        GDREGISTER_INTERNAL_CLASS(editor::commands::SelectPositionableCommand);
        GDREGISTER_INTERNAL_CLASS(editor::commands::SetLayerVisibilityCommand);
        GDREGISTER_INTERNAL_CLASS(editor::commands::SetPositionableVisibilityCommand);

        GDREGISTER_INTERNAL_CLASS(editor::PositionableItemListMetadata);
        GDREGISTER_INTERNAL_CLASS(editor::inspector::PositionableSelectionPane);
        GDREGISTER_INTERNAL_CLASS(editor::inspector::PositionableSetEditor);
        GDREGISTER_INTERNAL_CLASS(editor::inspector::LayersEditor);
        GDREGISTER_INTERNAL_CLASS(editor::IsometricEditorPlugin);
        GDREGISTER_INTERNAL_CLASS(editor::PositionableSetEditorPlugin);
        GDREGISTER_INTERNAL_CLASS(editor::IsometricTileEditorPlugin);
        EditorPlugins::add_by_type<editor::IsometricEditorPlugin>();
        EditorPlugins::add_by_type<editor::PositionableSetEditorPlugin>();
        EditorPlugins::add_by_type<editor::IsometricTileEditorPlugin>();
    }
#endif
}

void uninitialize_isometric_maps_module(ModuleInitializationLevel p_level) {
#ifdef TOOLS_ENABLED
    if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
        editor::PositionableScenesCacheManager::free();
        editor::PositionableSelectorManager::free();
    }
#endif

    if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
        Engine::get_singleton()->unregister_singleton(SERVER_SINGLETON_NAME);
        IsometricServer::terminate_server();
        IsometricStringNames::free();
    }
}

extern "C" {
GDExtensionBool GDE_EXPORT isometric_maps_library_init(
  GDExtensionInterfaceGetProcAddress p_get_proc_address,
  const GDExtensionClassLibraryPtr p_library,
  GDExtensionInitialization* r_initialization
) {
    GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

    init_obj.register_initializer(initialize_isometric_maps_module);
    init_obj.register_terminator(uninitialize_isometric_maps_module);
    init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

    return init_obj.init();
}
}

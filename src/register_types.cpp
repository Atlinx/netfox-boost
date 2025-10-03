#include "register_types.h"

#include <gdextension_interface.h>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

#include "netfox_boost.h"
#include "logger.h"
#include "property_cache.h"
#include "property_config.h"
#include "property_entry.h"
#include "property_history_buffer.h"
#include "property_snapshot.h"
#include "peer_visibility_filter.h"
#include "diff_history_encoder.h"
#include "redundant_history_encoder.h"
#include "snapshot_history_encoder.h"
#include "rollback_history_recorder.h"
#include "rollback_history_transmitter.h"

using namespace godot;

void initialize_gdextension_types(ModuleInitializationLevel p_level)
{
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
	GDREGISTER_CLASS(NetfoxBoost);
	GDREGISTER_CLASS(_NetfoxLogger);
	GDREGISTER_CLASS(PropertyCache);
	GDREGISTER_CLASS(_PropertyConfig);
	GDREGISTER_CLASS(PropertyEntry);
	GDREGISTER_CLASS(_PropertyHistoryBuffer);
	GDREGISTER_CLASS(_PropertySnapshot);
	GDREGISTER_CLASS(PeerVisibilityFilter);
	GDREGISTER_CLASS(_DiffHistoryEncoder);
	GDREGISTER_CLASS(_RedundantHistoryEncoder);
	GDREGISTER_CLASS(_SnapshotHistoryEncoder);
	GDREGISTER_CLASS(_RollbackHistoryRecorder);
	GDREGISTER_CLASS(_RollbackHistoryTransmitter);
}

void uninitialize_gdextension_types(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
}

extern "C"
{
	// Initialization
	GDExtensionBool GDE_EXPORT netfox_boost_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization)
	{
		GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);
		init_obj.register_initializer(initialize_gdextension_types);
		init_obj.register_terminator(uninitialize_gdextension_types);
		init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

		return init_obj.init();
	}
}
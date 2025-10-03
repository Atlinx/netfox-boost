#pragma once

#include <godot_cpp/godot.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

using namespace godot;

class _NetfoxLogger : public RefCounted {
	GDCLASS(_NetfoxLogger, RefCounted);
public:

	enum  {LOG_MIN, LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_MAX};

protected:
	static const int DEFAULT_LOG_LEVEL = LOG_DEBUG;

	static int log_level;
	static HashMap<String, int> module_log_level;

	static HashMap<int, Vector<Callable>> _tags;
	static Vector<Callable> _ordered_tags;
	static bool _static_initialized;

	String module;
	String name;

	const String level_prefixes[7] = {
		"", 
		"TRC", 
		"DBG", 
		"INF", 
		"WRN", 
		"ERR", 
		"", 
	};

	static void _bind_methods();

public:
	_NetfoxLogger() = default;
	~_NetfoxLogger() override = default;

	static void set_log_level(int p_level) { log_level = p_level; }
	static int get_log_level() { return log_level; }

	static void set_module_log_level(const Dictionary& p_log_levels);
	static Dictionary get_module_log_level();

	static Ref<_NetfoxLogger> for_netfox(String p_name);
	static Ref<_NetfoxLogger> for_noray(String p_name);
	static Ref<_NetfoxLogger> for_extras(String p_name);
	static Dictionary make_setting(String name);
	static void register_tag(Callable tag, int priority = 0);
	static void free_tag(Callable tag);
	static void _static_init();
	static Ref<_NetfoxLogger> _new(String p_module, String p_name);

	void _init(String p_module, String p_name);
	bool _check_log_level(int level);
	String _format_text(String text, Array values, int level);
	void _log_text(String text, Array values, int level);
	void trace(String text, Array values = Array());
	void debug(String text, Array values = Array());
	void info(String text, Array values = Array());
	void warning(String text, Array values = Array());
	void error(String text, Array values = Array());
};
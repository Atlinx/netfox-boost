#pragma once

#include <godot_cpp/godot.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

using namespace godot;

class _NetfoxLogger : public RefCounted {
	GDCLASS(_NetfoxLogger, RefCounted);
public:
	enum LogLevel {LOG_MIN, LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_MAX};

protected:
	static const LogLevel DEFAULT_LOG_LEVEL = LOG_DEBUG;

	static LogLevel log_level;
	static HashMap<String, LogLevel> module_log_level;
	static HashMap<int, Vector<Callable>> _tags;
	static Vector<Callable> _ordered_tags;

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

	static Ref<_NetfoxLogger> new_(String p_module, String p_name);

	static void set_log_level(LogLevel p_level) { log_level = p_level; }
	static LogLevel get_log_level() { return log_level; }

	static void set_module_log_level(const Dictionary& p_log_levels);
	static Dictionary get_module_log_level();

	static Ref<_NetfoxLogger> for_netfox(String p_name);
	static Ref<_NetfoxLogger> for_noray(String p_name);
	static Ref<_NetfoxLogger> for_extras(String p_name);
	static Dictionary make_setting(String name);
	static void register_tag(Callable tag, int priority = 0);
	static void free_tag(Callable tag);

	bool _check_log_level(LogLevel level);
	String _format_text(String text, Array values, LogLevel level);
	void _log_text(String text, Array values, LogLevel level);
	void trace(String text, Array values = Array());
	void debug(String text, Array values = Array());
	void info(String text, Array values = Array());
	void warning(String text, Array values = Array());
	void error(String text, Array values = Array());
};

VARIANT_ENUM_CAST(_NetfoxLogger::LogLevel);
#ifndef ISOMETRIC_MAPS_LOGGING_H
#define ISOMETRIC_MAPS_LOGGING_H

#include <core/error_macros.hpp>
#include <core/print_string.hpp>
#include <variant/string.hpp>

#define ISOMETRIC_MAPS_LOG_PREFIX "Isometric-Maps: "

#define LOG_VERBOSE(message) ::godot::print_verbose(::godot::String(ISOMETRIC_MAPS_LOG_PREFIX) + (message))

#define LOG_INFO(message) ::godot::print_line(::godot::String(ISOMETRIC_MAPS_LOG_PREFIX) + (message))

#define LOG_WARNING(message) WARN_PRINT(::godot::String(ISOMETRIC_MAPS_LOG_PREFIX) + (message))

#define LOG_ERROR(message) ERR_PRINT(::godot::String(ISOMETRIC_MAPS_LOG_PREFIX) + (message))

// Guards against API misuse and broken internal invariants that a correct project never triggers. They are compiled
// out of template_release builds, where the condition is not even evaluated: release favours speed over diagnostics.
#ifdef DEBUG_ENABLED
#define ISO_DEBUG_FAIL_COND_MSG(m_cond, m_msg) \
    ERR_FAIL_COND_MSG(m_cond, ::godot::String(ISOMETRIC_MAPS_LOG_PREFIX) + (m_msg))
#define ISO_DEBUG_FAIL_COND_V_MSG(m_cond, m_retval, m_msg) \
    ERR_FAIL_COND_V_MSG(m_cond, m_retval, ::godot::String(ISOMETRIC_MAPS_LOG_PREFIX) + (m_msg))
#else
#define ISO_DEBUG_FAIL_COND_MSG(m_cond, m_msg) ((void) 0)
#define ISO_DEBUG_FAIL_COND_V_MSG(m_cond, m_retval, m_msg) ((void) 0)
#endif

#endif// ISOMETRIC_MAPS_LOGGING_H

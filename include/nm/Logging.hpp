#ifndef NM_LOGGING_HPP
#define NM_LOGGING_HPP

#include <boost/core/null_deleter.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>

#define ANSI_BLUE "\033[34m"
#define ANSI_GREEN "\033[32m"
#define ANSI_RED "\033[31m"
#define ANSI_CLEAR "\033[0m"

namespace expr    = boost::log::expressions;
namespace logging = boost::log;

void logging_init(std::string fname, bool log_to_stderr = false)
{
    boost::shared_ptr<boost::log::sinks::text_ostream_backend> backend =
        boost::make_shared<boost::log::sinks::text_ostream_backend>();
    backend->add_stream(boost::shared_ptr<std::ostream>(new std::ofstream(fname, std::ios::ate)));
    if (log_to_stderr)
        backend->add_stream(boost::shared_ptr<std::ostream>(&std::cerr, boost::null_deleter()));

    backend->auto_flush(true);

    typedef boost::log::sinks::synchronous_sink<boost::log::sinks::text_ostream_backend> sink_t;
    boost::shared_ptr<sink_t> sink(new sink_t(backend));

    sink->set_formatter(
        [](boost::log::record_view const& record, logging::formatting_ostream& stream_ref) {
            if (record[expr::smessage] == "")
            {
                stream_ref << "";
                return;
            }

            auto result = std::time(nullptr);

            stream_ref << ANSI_RED << std::put_time(std::gmtime(&result), "%a %d-%m-%Y %H:%M:%S")
                       << ANSI_BLUE " [" << record[logging::trivial::severity] << "] " ANSI_GREEN
                       << record[expr::smessage] << ANSI_CLEAR;
        });

    boost::log::core::get()->add_sink(sink);

    boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);

    boost::log::add_common_attributes();
}

#endif

/** ==========================================================================
* 2013 by KjellKod.cc. This is PUBLIC DOMAIN to use at your own risk and comes
* with no warranties. This code is yours to share, use and modify with no
* strings attached and no restrictions or obligations.
*
* For more information see g3log/LICENSE or refer refer to http://unlicense.org
* ============================================================================*/
#pragma once

#include <string>
#include <memory>

#include "g3log/logmessage.hpp"
namespace g3 {

	class MyCustFileSink 
	{
	public:
		MyCustFileSink(const std::string &log_prefix, const std::string &log_directory, const std::string &logger_id = "g3log");
		virtual ~MyCustFileSink();

		void fileWrite(LogMessageMover message);
		std::string changeLogFile(const std::string &directory, const std::string &logger_id);
		std::string fileName();


	private:
		std::string _log_file_with_path;
		std::string _log_prefix_backup; // needed in case of future log file changes of directory
		std::unique_ptr<std::ofstream> _outptr;
		std::string _logger_id;
		std::string _log_directory;

		void addLogFileHeader();
		std::ofstream &filestream() 
		{
			return *(_outptr.get());
		}
		bool rotateLog();
		void fileWriteWithoutRotate(std::string message);
		void flushPolicy();
		void flush();
		std::string CustLogDetailsToString(const g3::LogMessage& msg);

		int _currentLogDay;
		int _lastLogDay;
	


		MyCustFileSink &operator=(const MyCustFileSink &) = delete;
		MyCustFileSink(const MyCustFileSink &other) = delete;

	};
} // g3


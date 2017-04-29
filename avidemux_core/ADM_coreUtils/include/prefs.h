#ifndef _preferences_h_
#define _preferences_h_

#include "ADM_coreUtils6_export.h"
#include "ADM_default.h"
#include <vector>
#define RC_OK     true
#define RC_FAILED false
#define NB_LAST_FILES 4

// <prefs_gen>
typedef enum {
#include "prefs2_list.h"
LAST_PREF
} options;
// </prefs_gen>

class ADM_COREUTILS6_EXPORT preferences {
	private:

//		const char * get_str_min(options option);
//		const char * get_str_max(options option);
		void setFile(const std::string &file, std::string *file1, int maxFiles);

	public:
		preferences();
		~preferences();
		bool load();
		bool save();
		bool get(options option, uint32_t *);
		bool get(options option, int32_t  *);
		bool get(options option, float *val);
//		bool get(options option, char **val);
		bool get(options option, bool *val);
                bool get(options option, std::string &v);


		bool set(options option, const uint32_t val);
		bool set(options option, const int32_t  val);
		bool set(options option, const float    val);
//		bool set(options option, const char    *val);
		bool set(options option, const bool     val);
		bool set(options option, const std::string &v);

		bool  set_lastfile(const char* file);
		bool  set_lastprojectfile(const char* file);
                bool  clear_lastfiles(void);
                bool  clear_lastprojects(void);
		std::vector< std::string>get_lastfiles(void);
		std::vector< std::string>get_lastprojectfiles(void);		
};

extern ADM_COREUTILS6_EXPORT preferences *prefs;
ADM_COREUTILS6_EXPORT bool initPrefs(  void );
ADM_COREUTILS6_EXPORT bool destroyPrefs(  void );
#endif

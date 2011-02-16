changequote(<<,>>)dnl
include(<<project.m4>>)dnl
@set PROJECT.name            __PROJECT_name
@set PROJECT.name.lower      __PROJECT_name_lower
@set PROJECT.name.upper      __PROJECT_name_upper
@set PROJECT.name.formal     __PROJECT_name_formal
@set PROJECT.url.website     __PROJECT_url_website
@set PROJECT.url.downloads   __PROJECT_url_downloads
@set PROJECT.url.discussion  __PROJECT_url_discussion
@set PROJECT.irc             __PROJECT_irc
@set PROJECT.bugreport       patsubst(__PROJECT_bugreport,@,@@)
@set PROJECT.version         __PROJECT_version
@set PROJECT.version.hex     __PROJECT_version_hex
@set PROJECT.version.major   __PROJECT_version_major
@set PROJECT.version.minor   __PROJECT_version_minor
@set PROJECT.version.point   __PROJECT_version_point
@set PROJECT.repo.url        __PROJECT_repo_url
@set PROJECT.repo.branch     __PROJECT_repo_branch
@set PROJECT.repo.root       __PROJECT_repo_root
@set PROJECT.repo.uuid       __PROJECT_repo_uuid
@set PROJECT.repo.rev        __PROJECT_repo_rev
@set PROJECT.repo.date       __PROJECT_repo_date
@set PROJECT.repo.type       __PROJECT_repo_type
@set PROJECT.build           __PROJECT_build

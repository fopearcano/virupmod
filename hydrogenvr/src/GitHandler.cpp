/*
    Copyright (C) 2024 Florian Cabot <florian.cabot@hotmail.fr>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "GitHandler.hpp"

void GitHandler::Repository::Remote::fetch() const
{
#ifdef GIT
	git_remote_fetch(remote, nullptr, nullptr, nullptr);
#endif
}

// not static to make sure git2 was initialized
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void GitHandler::clone(QString const& url, QString const& path)
{
#ifdef GIT
	git_repository* repo = nullptr;
	git_clone(&repo, url.toLatin1().data(), path.toLatin1().data(), nullptr);
	git_repository_free(repo);
	repo = nullptr;
#endif
}

// RAII DEFS

GitHandler::GitHandler()
{
#ifdef GIT
	git_libgit2_init();
#endif
}

GitHandler::~GitHandler()
{
#ifdef GIT
	git_libgit2_shutdown();
#endif
}

GitHandler::Repository::Repository(QString const& path)
{
#ifdef GIT
	git_repository_open(&repo, path.toLatin1().data());
#endif
}

GitHandler::Repository::~Repository()
{
#ifdef GIT
	git_repository_free(repo);
	repo = nullptr;
#endif
}

#ifdef GIT
GitHandler::Repository::Remote::Remote(git_repository& repo,
                                       QString const& name)
{
	git_remote_lookup(&remote, &repo, name.toLatin1().data());
}
#endif

GitHandler::Repository::Remote::~Remote()
{
#ifdef GIT
	git_remote_free(remote);
	remote = nullptr;
#endif
};

#ifdef GIT
GitHandler::Repository::Reference::Reference(git_repository& repo,
                                             QString const& name)
{
	git_reference_lookup(&ref, &repo, name.toLatin1().data());
}
#endif

GitHandler::Repository::Reference::~Reference()
{
#ifdef GIT
	git_reference_free(ref);
	ref = nullptr;
#endif
};

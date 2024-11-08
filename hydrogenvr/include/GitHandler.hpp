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

#ifndef GITHANDLER_HPP
#define GITHANDLER_HPP

#ifdef GIT
#include <git2.h>
#endif

#include <QString>

class GitHandler
{
  public:
	class Repository
	{
	  public:
		class Remote
		{
		  public:
			Remote(Remote&&)                 = default;
			Remote(Remote const&)            = delete;
			Remote& operator=(Remote const&) = delete;
			Remote& operator=(Remote&&)      = delete;
			~Remote();

			void fetch() const;

		  private:
#ifdef GIT
			Remote(git_repository& repo, QString const& name);
			git_remote* remote = nullptr;
#endif
		};

		class Reference
		{
		  public:
			Reference(Reference&&)                 = default;
			Reference(Reference const&)            = delete;
			Reference& operator=(Reference const&) = delete;
			Reference& operator=(Reference&&)      = delete;
			~Reference();

		  private:
#ifdef GIT
			Reference(git_repository& repo, QString const& name);
			git_reference* ref = nullptr;
#endif
		};

		Repository(Repository&&)                 = default;
		Repository(Repository const&)            = delete;
		Repository& operator=(Repository const&) = delete;
		Repository& operator=(Repository&&)      = delete;
		~Repository();

	  private:
		explicit Repository(QString const& path);

#ifdef GIT
		git_repository* repo = nullptr;
#endif
	};

	GitHandler();
	GitHandler(GitHandler&&)                 = default;
	GitHandler(GitHandler const&)            = delete;
	GitHandler& operator=(GitHandler const&) = delete;
	GitHandler& operator=(GitHandler&&)      = delete;
#ifdef GIT
	static bool isEnabled() { return true; };
	void clone(QString const& url, QString const& path);
#else
	static bool isEnabled() { return false; };
#endif
	~GitHandler();
};

#endif // GITHANDLER_HPP

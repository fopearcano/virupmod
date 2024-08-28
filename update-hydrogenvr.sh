#!/bin/bash -e

update() {

	### fetch code
	git fetch --all
	if ! git remote show hydrogenvr &> /dev/null; then
		git remote add hydrogenvr https://gitlab.com/Dexter9313/hydrogenvr.git
	fi
	if [ $# -eq 0 ]
	then
		HASH=$(git ls-remote hydrogenvr -h refs/heads/master | cut -f1)
		git pull --squash -X theirs hydrogenvr master --allow-unrelated-histories
	else
		HASH=$1
		CURRENT_BRANCH=$(git branch | grep \* | cut -d ' ' -f2)
		git checkout --orphan update_hydrogenvr
		rm -rf ./*
		git reset --hard
		git pull hydrogenvr master
		git reset --hard $HASH
		git checkout $CURRENT_BRANCH
		git merge --squash -X theirs update_hydrogenvr --allow-unrelated-histories
		git branch -D update_hydrogenvr
	fi

	### construct changelog
	PREV_HVR_COMMIT=$(git log --oneline | grep "Update HydrogenVR to " | head -n 1 | cut -d ' ' -f5)
	CHANGELOG=$(git log $PREV_HVR_COMMIT..$HASH --oneline)
	COMMIT_MESSAGE="Update HydrogenVR to $HASH\n\nfrom $PREV_HVR_COMMIT\n\nCHANGELOG:\n$CHANGELOG"
	COMMIT_MESSAGE=$(echo -e "$COMMIT_MESSAGE")
	git commit -m "$COMMIT_MESSAGE" -e

	. ./project_directory.conf || PROJECT_DIRECTORY=hydrogenvr/example
	if [[ $(grep PROJECT hydrogenvr/example/build.conf | wc -l) -ne $(grep PROJECT $PROJECT_DIRECTORY/build.conf | wc -l) ]]
	then
		echo "WARNING:"
		echo "Your build.conf seems outdated, please compare it with hydrogenvr/example/build.conf."
	fi
}

test -n "$(git status --porcelain --untracked-files=no)" && echo "Please cleanup working directory (at least stash your work, make sure no HydrogenVR file is edited, even in stash)." && exit 0
update $1


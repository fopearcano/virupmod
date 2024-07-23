#!/bin/bash -e

SUBDIR=$1

apt-get update && apt-get install -y cmake doxygen graphviz
mkdir -p build
cd build
cmake .. -DDOC_ONLY=true
make doc
mv html ..
cd ..
mkdir public
mv html public/$SUBDIR

# GENERATE GLOBAL INDEX.HTML

. ./project_directory.conf || PROJECT_DIRECTORY=hydrogenvr/example
. ./${PROJECT_DIRECTORY}/build.conf

# Create an index.html file in the public directory
INDEX_FILE="public/index.html"
# Base URL for the documentation
BASE_URL=$2

echo "Base URL: $BASE_URL"

# Start the HTML file
echo "<!DOCTYPE html>" > $INDEX_FILE
echo "<html lang='en'>" >> $INDEX_FILE
echo "<head>" >> $INDEX_FILE
echo "    <meta charset='UTF-8'>" >> $INDEX_FILE
echo "    <meta name='viewport' content='width=device-width, initial-scale=1.0'>" >> $INDEX_FILE
echo "    <title>$PROJECT_NAME Documentation</title>" >> $INDEX_FILE
echo "    <style>" >> $INDEX_FILE
echo "        body { font-family: Arial, sans-serif; }" >> $INDEX_FILE
echo "        ul { list-style-type: none; }" >> $INDEX_FILE
echo "        li { margin: 5px 0; }" >> $INDEX_FILE
echo "    </style>" >> $INDEX_FILE
echo "</head>" >> $INDEX_FILE
echo "<body>" >> $INDEX_FILE
echo "    <h1>$PROJECT_NAME Documentation</h1>" >> $INDEX_FILE
echo "    <h2>Branches</h2>" >> $INDEX_FILE
echo "    <ul>" >> $INDEX_FILE

# Function to check if a URL exists
url_exists() {
    curl --head --silent --fail $1 > /dev/null
    return $?
}

# Loop through all branches
for branch in $(git branch -r | grep -v '\->' | awk '{print $1}' | sed 's#origin/##'); do
    # Skip special branch names if necessary
    if [[ "$branch" != "HEAD" ]]; then
        # Construct the URL for the branch
        branch_url="$BASE_URL/$branch/"
        # Check if the URL exists or if it is the current branch being processed
        if [[ "$branch" == "$SUBDIR" ]] || url_exists $branch_url; then
            # Add a list item for each valid branch
            echo "        <li><a href='$branch/'>$branch</a></li>" >> $INDEX_FILE
        fi
    fi
done

echo "    </ul>" >> $INDEX_FILE
echo "    <h2>Tags</h2>" >> $INDEX_FILE
echo "    <ul>" >> $INDEX_FILE

# Loop through all tags
for tag in $(git tag); do
    # Construct the URL for the tag
    tag_url="$BASE_URL/$tag/"
    # Check if the URL exists or if it is the current tag being processed
    if [[ "$tag" == "$SUBDIR" ]] || url_exists $tag_url; then
        # Add a list item for each valid tag
        echo "        <li><a href='$tag/'>$tag</a></li>" >> $INDEX_FILE
    fi
done

# End the HTML file
echo "    </ul>" >> $INDEX_FILE
echo "</body>" >> $INDEX_FILE
echo "</html>" >> $INDEX_FILE

echo "Index file generated at $INDEX_FILE"

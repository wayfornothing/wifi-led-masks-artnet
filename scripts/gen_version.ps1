# Get the latest Git commit hash
$GIT_HASH = git rev-parse --short HEAD 2>$null
if (-not $GIT_HASH) { $GIT_HASH = "0" }

# Get the current Git branch name
$GIT_BRANCH = git rev-parse --abbrev-ref HEAD 2>$null
if (-not $GIT_BRANCH) { $GIT_BRANCH = "unknown" }

# Get the latest Git tag
$GIT_TAG = git describe --tags --abbrev=0 2>$null
if (-not $GIT_TAG) { $GIT_TAG = "0.0.0" }

# Remove leading 'v' if present
$GIT_TAG_CLEAN = $GIT_TAG.TrimStart("v")

# Extract major, minor, and patch versions
$MAJOR = 0
$MINOR = 0
$PATCH = 0

$parts = $GIT_TAG_CLEAN -split "\."
if ($parts.Length -ge 1) { $MAJOR = [int]$parts[0] }
if ($parts.Length -ge 2) { $MINOR = [int]$parts[1] }
if ($parts.Length -ge 3) { $PATCH = [int]$parts[2] }

# Output file
$OUTPUT_FILE = "include/version.h"

@"
#pragma once

#define GIT_HASH        ("$GIT_HASH")
#define GIT_BRANCH      ("$GIT_BRANCH")
#define GIT_TAG         ("$GIT_TAG_CLEAN")
#define GIT_TAG_MAJOR   ($MAJOR)
#define GIT_TAG_MINOR   ($MINOR)
#define GIT_TAG_PATCH   ($PATCH)
"@ | Set-Content -Encoding UTF8 $OUTPUT_FILE

Write-Host "Generated $OUTPUT_FILE successfully."

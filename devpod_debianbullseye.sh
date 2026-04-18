#!/bin/sh
devpod up . --ide none --id adm-bullseye --devcontainer-path .devcontainer_bullseye/devcontainer.json $@

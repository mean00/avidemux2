/*
 * Copyright 2012 Google, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Google Author(s): Behdad Esfahbod, Maysum Panju, Wojciech Baranowski
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "glyphy-common.hh"

/*
 * Shader source code
 */

/* TODO path separator */
#define SHADER_PATH(File) PKGDATADIR "/" File

#include "glyphy-common-glsl.h"
#include "glyphy-sdf-glsl.h"

const char * glyphy_common_shader_source (void) { return glyphy_common_glsl; }
const char * glyphy_sdf_shader_source (void) { return glyphy_sdf_glsl; }

const char * glyphy_common_shader_source_path (void) {  return SHADER_PATH ("glyphy-common.glsl"); }
const char * glyphy_sdf_shader_source_path (void) {  return SHADER_PATH ("glyphy-sdf.glsl"); }

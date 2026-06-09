#
# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: Apache-2.0
#

use_relative_paths = True

git_dependencies = 'DEPS'

vars = {
  'edge_git': 'https://microsoft.visualstudio.com/edge/_git',
  'edge_git_suffix': '',
  'oneds_modules_revision': 'c637015fbbe904ed556d27e3b9072f6f2a5ee401',
}

deps = {
  'lib/modules': {
    'url': Var('edge_git') + '/microsoft.cpp_client_telemetry_modules' + Var('edge_git_suffix') + '@' + Var('oneds_modules_revision'),
    'condition': 'checkout_ms_src_internal'
  },
}
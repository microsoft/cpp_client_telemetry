# Copyright (C) Microsoft Corporation. All rights reserved.

use_relative_paths = True

git_dependencies = 'DEPS'

vars = {
  'edge_git': 'https://microsoft.visualstudio.com/edge/_git',
  'edge_git_suffix': '',
  'oneds_modules_revision': 'fe090b0c324e88d602e67edafd5919cb65190e81',
}

deps = {
  'lib/modules': {
    'url': Var('edge_git') + '/microsoft.cpp_client_telemetry_modules' + Var('edge_git_suffix') + '@' + Var('oneds_modules_revision'),
    'condition': 'checkout_ms_src_internal'
  },
}

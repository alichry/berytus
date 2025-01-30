const { dirname } = require('path');

module.exports = {
    ...require('./craco.config'),
    webpack: {
      configure: (webpackConfig) => {
        webpackConfig = require('./craco.config').webpack.configure(webpackConfig);
        //webpackConfig.stats = 'verbose';
        webpackConfig.entry = {
          // entry must have a `main` key, I do not know why.
          main: { import: './src/workers/main.ts', dependOn: ['dexie-lib'] },
          //worker2: './src/workers/worker2.ts',
          'dexie-lib': ['dexie']
        };
        // webpackConfig.optimization.minimize = false;
        // webpackConfig.optimization.minimizer = [];
        return webpackConfig;
      }
    }
};
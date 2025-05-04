module.exports = {
    ...require('./craco.config'),
    webpack: {
      configure: (webpackConfig) => {
        webpackConfig = require('./craco.config').webpack.configure(webpackConfig);
        //webpackConfig.stats = 'verbose';
        webpackConfig.entry = {
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
//const {CracoAliasPlugin} = require('react-app-alias-ex')
const { CracoAliasPlugin, configPaths } = require('react-app-rewire-alias/lib/aliasDangerous')
const { ProvidePlugin, optimize: { LimitChunkCountPlugin }, web } = require('webpack');
const MiniCssExtractPlugin = require('mini-css-extract-plugin');

const options = {} // default is empty for most cases
const aliasMap = configPaths('./tsconfig.paths.json')

module.exports = {
  plugins: [
    {
      plugin: CracoAliasPlugin,
      options: {
        aliasMap
      }
    },
    {
      plugin: {
        overrideWebpackConfig: ({ webpackConfig }) => {
          const miniCssPlugin = webpackConfig.plugins.find(plugin => plugin instanceof MiniCssExtractPlugin);
          if (!miniCssPlugin) {
            // in dev mode, no miniCssPlugin is defined.
            return webpackConfig;
          }
          miniCssPlugin.options.filename = 'static/css/[name].css';
          return webpackConfig;
        },
      },
      options: {}
    }
  ],
  webpack: {
    configure: (webpackConfig) => {
      //webpackConfig.stats = 'verbose';
      /* polyfill for jsrp client */
      webpackConfig.resolve.fallback = webpackConfig.resolve.fallback || {};
      webpackConfig.resolve.fallback.buffer = require.resolve("buffer/");
      webpackConfig.resolve.fallback.stream = require.resolve("stream-browserify");
      webpackConfig.resolve.fallback.process = require.resolve('process/browser');
      webpackConfig.resolve.fallback.crypto = require.resolve('crypto-browserify');
      webpackConfig.resolve.fallback.assert = require.resolve('assert/');
      /* asn1.js (via crypto-browserify) does an optional require('vm')
       * inside a try/catch and falls back when it fails; an empty module
       * takes the same fallback path and silences the build warning. */
      webpackConfig.resolve.fallback.vm = false;
      /* Some dependencies (e.g. fast-srp-hap) ship source maps that point to
       * .ts sources they don't publish. With CI=true these warnings fail the
       * build; they only affect debugging of node_modules. */
      webpackConfig.ignoreWarnings = (webpackConfig.ignoreWarnings || []).concat(
        (warning) =>
          typeof warning.message === 'string' &&
          warning.message.includes('Failed to parse source map') &&
          /node_modules/.test(warning.message)
      );
      webpackConfig.plugins.push(new ProvidePlugin({
        Buffer: ['buffer', 'Buffer'],
        process: require.resolve('process/browser') // needed for nextTick
      }));
      // webpackConfig.plugins.push(new LimitChunkCountPlugin({
      //   maxChunks: 1
      // }));
      // webpackConfig.plugins.push(new NodePolyfillPlugin({
      //   excludeAliases: ['console'],
      // }));
      // webpackConfig.optimization.minimize = false;
      // webpackConfig.optimization.minimizer = [];
      //webpackConfig.mode ='production',
      webpackConfig.optimization = { usedExports: true }
      webpackConfig.output.filename = 'static/js/[name].js'; /* default is static/js/[name].[contenthash:8].js */
      webpackConfig.output.chunkFilename = 'static/js/[name].chunk.js';
      webpackConfig.optimization.minimize = true;
      webpackConfig.optimization.splitChunks = {
        name: (module, chunks, cacheGroupKey) => {
          const allChunksNames = chunks.map((chunk) => chunk.name).join('-');
          return allChunksNames;
        },
      };
      return webpackConfig;
    }
  }
}
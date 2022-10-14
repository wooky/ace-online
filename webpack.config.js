const path = require('path');
const CopyPlugin = require("copy-webpack-plugin");
const HtmlWebpackPlugin = require('html-webpack-plugin')
const { WebpackTexturePackerPlugin } = require('webpack-texture-packer');

module.exports = {
  mode: 'development',
  entry: './wasm/index.js',
  output: {
    filename: 'ace-online.js',
    path: path.resolve(__dirname, 'build', 'dist'),
  },
  module: {
    rules: [
      {
        test: /\.in$/,
        loader: 'csv-loader',
        options: {
          delimiter: " ",
          skipEmptyLines: true,
        }
      },
    ],
  },
  resolve: {
    alias: {
      "@": path.resolve(__dirname, "wasm/"),
      "@build": path.resolve(__dirname, "build/"),
    },
    fallback: {
      "crypto": false,
      "path": false,
      "fs": false,
    },
  },
  plugins: [
    new CopyPlugin({
      patterns: [
        "./build/ace-online.wasm",
      ],
    }),
    new WebpackTexturePackerPlugin({
      rootDir: __dirname,
      items: [
        {
          name: "ace-texture",
          source: [
            "games",
            "lib/png",
          ],
          packerOptions: {
            allowRotation: false,
            prependFolderName: false,
            removeFileExtension: true,
          },
        },
      ],
    }),
    new HtmlWebpackPlugin({
      template: "wasm/index.html",
    }),
  ],
};

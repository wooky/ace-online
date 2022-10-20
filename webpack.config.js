const path = require('path');
const CopyPlugin = require("copy-webpack-plugin");
const HtmlWebpackPlugin = require('html-webpack-plugin')

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
        test: /\.(png|svg|jpg|jpeg|gif)$/i,
        type: 'asset/resource',
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
        "./wasm/ace-online.css",
        "./wasm/icon.png",
        "./wasm/manifest.webmanifest",
      ],
    }),
    new HtmlWebpackPlugin({
      template: "wasm/index.html",
    }),
  ],
};

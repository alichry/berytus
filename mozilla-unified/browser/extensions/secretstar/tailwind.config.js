const colors = require('tailwindcss/colors');
const defaultColors = Object.keys(colors)
  .reduce((res, name) => {
    switch (name) {
      case "lightBlue":
      case "warmGray":
      case "trueGray":
      case "coolGray":
      case "blueGray":
        return res;
      default:
        res[name] = colors[name];
        return res;
    }
  }, {});

/** @type {import('tailwindcss').Config} */

module.exports = {
  content: [
    "./src/**/*.{js,jsx,ts,tsx}",
  ],
  theme: {
    fontFamily: {
      'display': ['M PLUS 1p'],
      'value': ['Inter'],
      'code': ['Monaco', 'Menlo', 'Ubuntu Mono', 'Consolas', 'source-code-pro', 'monospace']
    },
    colors: {
      ...defaultColors,
      default: "#FFF3F3",
      defaultPurple: "#5308F3",
      defaultPurpleDark: "#260078"
    },
    extend: {
      colors: defaultColors
    },
  },
  plugins: [],
}

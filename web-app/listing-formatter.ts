#!/usr/bin/env deno run -A
import * as prettier from 'npm:prettier';
import prettierPluginBabel from 'npm:prettier/plugins/babel';
import prettierPluginEstree from 'npm:prettier/plugins/estree';

const code = prompt("Please enter the code you want to format.");

console.log("Ok, here it formated.");
console.log("--------------------------");

const formatted = await prettier.format(code, {
    //parser: 'babel',
    parser: 'babel',
    plugins: [prettierPluginBabel, prettierPluginEstree],
    printWidth: 75,
    tabWidth: 4,
    semi: true
});

console.log(formatted);
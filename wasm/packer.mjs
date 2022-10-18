import { parse as parseCsv } from "csv-parse/sync";
import { packAsync } from "free-tex-packer-core";
import { globby } from "globby";
import Mustache from "mustache";
import * as fs from "node:fs";

if (process.argv.length != 5) {
  console.error(`Usage: ${process.argv0} ${process.argv[1]} <images.in> <output.c> <textures.png>`);
  process.exit(1);
}

const imagesMetaFilename = process.argv[2];
const cFilename = process.argv[3];
const textureFilename = process.argv[4];

const imagesMeta = parseCsv(fs.readFileSync(imagesMetaFilename), {
  delimiter: " ",
  relax_column_count: true,
  skip_empty_lines: true,
});

const images = (await globby(["games/*.png", "lib/png/*.png"])).map(path => ({
  path,
  contents: fs.readFileSync(path),
}));

const files = await packAsync(images, {
  allowRotation: false,
  prependFolderName: false,
  removeFileExtension: true,
  packer: "OptimalPacker",
});
const jsonFile = JSON.parse(files[0].buffer.toString());
const textureFile = files[1].buffer;

const texturePromise = fs.promises.writeFile(textureFilename, textureFile);

const combinedTextures = combineTextures(jsonFile, imagesMeta);
const cPromise = writeCFile(combinedTextures, cFilename);

Promise.all([texturePromise, cPromise]);

/**
 * @param {object} aceTextures
 * @return {object}
 */
function combineTextures(aceTextures, imagesMeta) {
  const images = {};
  for (const frame in aceTextures.frames) {
    const filenameParts = frame.split(".");
    const filename = filenameParts[0];
    if (!(filename in images)) {
      let across = 1, down = 1;
      const row = imagesMeta.find(e => e[0] == filename);
      if (row && row.length == 3) {
        across = row[1];
        down = row[2];
      }
      images[filename] = {
        filename,
        across,
        down,
        frames: []
      };
    }
    images[filename].frames.push(aceTextures.frames[frame].frame);
  }

  return Object.values(images);
}

async function writeCFile(textures, cFilename) {
  const templateFile = (await fs.promises.readFile("wasm/imagelib.c.mustache")).toString();
  const cFile = Mustache.render(templateFile, { textures });
  await fs.promises.writeFile(cFilename, cFile);
}

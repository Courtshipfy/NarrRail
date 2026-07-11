import { importFromYAML } from "../utils/yaml-importer.js";
import { buildYAMLString } from "../utils/yaml-exporter.js";
import { validateStory } from "../utils/validation.js";

export function parseStoryFromYAML(yamlText) {
  return importFromYAML(yamlText);
}

export function serializeStoryToYAML(story) {
  return buildYAMLString(
    story?.nodes || [],
    story?.edges || [],
    story?.variables || [],
    story?.meta || {},
  );
}

export function validateStoryDocument(story, context = {}) {
  return validateStory(
    story?.nodes || [],
    story?.edges || [],
    story?.meta || {},
    context.variables || story?.variables || [],
  );
}

export { buildYAMLString, importFromYAML, validateStory };

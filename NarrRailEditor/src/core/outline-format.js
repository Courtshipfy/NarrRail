import {
  buildEmptyRailData,
  buildRailYAMLString,
  importRailFromYAML,
} from "../utils/rail-yaml.js";
import { validateRail } from "../utils/rail-validation.js";

export function createEmptyOutline(outlineId = "main_story") {
  return buildEmptyRailData(outlineId);
}

export function parseOutlineFromYAML(yamlText) {
  return importRailFromYAML(yamlText);
}

export function serializeOutlineToYAML(outline) {
  return buildRailYAMLString(
    outline?.nodes || [],
    outline?.edges || [],
    outline?.meta || {},
  );
}

export function validateOutlineDocument(outline, context = {}) {
  return validateRail(
    outline?.nodes || [],
    outline?.edges || [],
    outline?.meta || {},
    {
      storyEntries: context.storyEntries || [],
      variables: context.variables || [],
    },
  );
}

export {
  buildEmptyRailData,
  buildRailYAMLString,
  importRailFromYAML,
  validateRail,
};

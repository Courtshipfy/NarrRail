#!/usr/bin/env node

import fs from "node:fs";
import path from "node:path";

const SKILL_ROOT = path.resolve(new URL("..", import.meta.url).pathname);

function usage() {
  console.error(
    "Usage: node prepare-ai-conversion.mjs <manuscript-file> <output-packet.md> [--profile <profile.md>]",
  );
}

function parseArgs(argv) {
  const args = [...argv];
  const inputPath = args.shift();
  const outputPath = args.shift();
  let profilePath = path.join(SKILL_ROOT, "references", "project-profile-template.md");

  while (args.length > 0) {
    const key = args.shift();
    if (key === "--profile") {
      profilePath = args.shift();
      continue;
    }
    throw new Error(`Unknown argument: ${key}`);
  }

  if (!inputPath || !outputPath) {
    usage();
    process.exit(1);
  }

  return { inputPath, outputPath, profilePath };
}

function readText(filePath) {
  return fs.readFileSync(filePath, "utf8");
}

function buildPacket({ manuscriptPath, manuscript, profilePath, profile, format, policy }) {
  return `# NarrRail Story Conversion Packet

Return only valid NarrRail .nrstory YAML unless the user explicitly asks for notes.

## Project Profile (${path.basename(profilePath)})

${profile.trim()}

## NarrRail Format

${format.trim()}

## Conversion Policy

${policy.trim()}

## Source Manuscript (${path.basename(manuscriptPath)})

\`\`\`text
${manuscript.trim()}
\`\`\`

## Final Instruction

Read the project profile and source manuscript first. Infer the story structure
from the writer's actual format. Then produce a valid NarrRail .nrstory YAML file
compatible with schemaVersion 1.
`;
}

function main() {
  const { inputPath, outputPath, profilePath } = parseArgs(process.argv.slice(2));
  const manuscript = readText(inputPath);
  const profile = readText(profilePath);
  const format = readText(path.join(SKILL_ROOT, "references", "nrstory-format.md"));
  const policy = readText(path.join(SKILL_ROOT, "references", "conversion-policy.md"));
  const packet = buildPacket({
    manuscriptPath: inputPath,
    manuscript,
    profilePath,
    profile,
    format,
    policy,
  });

  fs.mkdirSync(path.dirname(outputPath), { recursive: true });
  fs.writeFileSync(outputPath, packet, "utf8");
  console.log(`Wrote ${outputPath}`);
}

main();


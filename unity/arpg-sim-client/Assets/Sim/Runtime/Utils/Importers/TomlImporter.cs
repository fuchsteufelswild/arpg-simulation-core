using UnityEngine;
using UnityEditor.AssetImporters;
using System.IO;

[ScriptedImporter(1, "toml")]
public class TomlImporter : ScriptedImporter
{
    public override void OnImportAsset(AssetImportContext ctx)
    {
        var text = File.ReadAllText(ctx.assetPath);
        var subAsset = new TextAsset(text);

        ctx.AddObjectToAsset("main", subAsset);
        ctx.SetMainObject(subAsset);
    }
}
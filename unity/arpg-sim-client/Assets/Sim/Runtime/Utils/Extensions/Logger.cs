using System.Diagnostics;

namespace Sim.Utils
{
    public static class Logger
    {
        [Conditional("UNITY_EDITOR"), Conditional("DEVELOPMENT_BUILD")]
        public static void Info(string message)
        {
            UnityEngine.Debug.Log(message);
        }

        [Conditional("UNITY_EDITOR")]
        public static void Verbose(string message)
        {
            UnityEngine.Debug.Log($"<color=grey>[VERBOSE]</color> {message}");
        }
    }
}
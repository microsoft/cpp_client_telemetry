using System;
using Android.Content;
using Java.Lang;

namespace Microsoft.Applications.Events
{
    public partial class LogManager
    {
        public static ILogger Initialize(Context context, string tenantToken)
        {
            if (context == null)
            {
                throw new ArgumentNullException(nameof(context), "context cannot be null");
            }

            JavaSystem.LoadLibrary("maesdk");
            HttpClient httpClient = new HttpClient(context);
            OfflineRoom.ConnectContext(context);

            return Initialize(tenantToken);
        }
    }
}
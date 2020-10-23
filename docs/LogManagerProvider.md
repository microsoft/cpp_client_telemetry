# LogManagerProvider

The ```LogManagerProvider``` class supplies instances of ```ILogManager```. It is used by the static ```Initialize()``` methods in ```LogManagerBase<>``` to set the private pointer to its singleton implementation (and this includes the default ```LogManager``` class derived from ```LogManagerBase<>```). It can be used without ```LogManagerBase<>``` to get an instance pointer directly.

## ILogConfiguration host and name

```LogManagerProvider``` uses two parameters from ```ILogConfiguration``` for its method ```CreateLogManager(ILogConfiguration &, status_t)```, the *host* and *name* parameters (both strings). The *name* parameter is accessed as ```config[CFG_STR_FACTORY_NAME]```. The *host* parameter is at ```config[CFG_MAP_FACTORY_CONFIG][CFG_STR_FACTORY_HOST]```. These combine to determine which log manager instance you get when you call ```CreateLogManager```.

### Exclusive Log Manager

When the *host* value is empty, you will get an **exclusive** instance. Which instance you get is determined by *name*. ```CreateLogManager``` will return the same instance for a given *name* whenever it is called with an empty *host*. The default configuration does not define name, so all instances of LogManagerBase<> will by default be the same log manager (with a *name* of "").

### Shared Log Manager

When the *host* value is not empty, you will get a **shared** instance. *host* may either be "\*" (any host, the wildcard host) or an arbitrary host name.

#### Any Host

When *host* is "\*", if there are already any shared instances, ```CreateLogManager``` returns the first shared instance in its map (and this *name* is added to that instances list of names).
Otherwise, ```CreateLogManager``` creates a new shared instance for the wildcard "\*" host and returns it.

#### Specific Host

When *host* is not a wildcard "\*", and a shared instance for that *host* exists, ```CreateLogManager``` returns it. Otherwise, if there is a log manager for the wildcard "\*" host, ```CreateLogManager``` repurposes it to be the instance for this named host and the existing wildcard hosts, and returns that. If neither of those conditions is true, we don't need to repurpose a wildcard instance and we don't have a specified-host instance, so ```CreateLogManager``` creates the new instance and returns it.

## Exclusive and Shared

Exclusive log managers don't have a host identifier. No shared log manager will ever get a pointer to an exclusive manager instance. All exclusive log managers with the same name (including the default blank name) are the same log manager (so they are, so to speak, shared). Shared log managers are differentiated by *host* rather than *name*, and can either be wildcard (use any log manager) or specific-host. One shared specific-host log manager serves all the wildcard log manager requests. In the absence of a shared specific-host log manager, all wildcard-host log managers share one instance (which will be
converted to specific-host as soon as the first shared specific-host manager is requested).

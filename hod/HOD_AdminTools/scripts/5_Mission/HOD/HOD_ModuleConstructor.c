// HOD Admin Tools - registers HOD modules with COT's module manager
modded class JMModuleConstructor
{
	override void RegisterModules(out TTypenameArray modules)
	{
		super.RegisterModules(modules);
		modules.Insert(HOD_ObjectBuilderModule);
		modules.Insert(HOD_XMLSearchModule);
	}
}

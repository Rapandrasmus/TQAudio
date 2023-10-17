def can_build(env, platform):
    env.module_add_dependencies("tqaudio", ["vorbis"])
    return True

def configure(env):
    pass
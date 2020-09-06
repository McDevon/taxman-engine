#include "render_context.h"
#include "platform_adapter.h"
#include <string.h>

void render_context_destroy(void *value)
{
}

char *render_context_describe(void *value)
{
    return platform_strdup("[]");
}

BaseType RenderContextType = { "RenderContext", &render_context_destroy, &render_context_describe };

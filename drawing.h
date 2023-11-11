#pragma once

#include <mymaths.h>
#include <mycolors.h>
#include <SDL.h>

SDL_Renderer *renderer = nullptr;
SDL_Window *window = nullptr;

void drawColor(Color color){
    SDL_SetRenderDrawColor(renderer, color.getRi(), color.getGi(), color.getBi(), color.getAi());
}
void drawPoint(int x, int y){
    SDL_RenderDrawPoint(renderer, x, y);
}
void drawPoint(Pos pos){
    SDL_RenderDrawPoint(renderer, pos.x, pos.y);
}
void drawLine(int x1, int y1, int x2, int y2){
    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
}
void drawLine(Pos pos1, Pos pos2){
    SDL_RenderDrawLine(renderer, pos1.x, pos1.y, pos2.x, pos2.y);
}
void drawBezier(Pos pos1, Pos dir1, Pos pos2, Pos dir2){
    int n = (pos1 - pos2).abs() / 2;
    Pos p0, p1, p2, last_pos = pos1;
    for (double i = 1.0 / n; i <= 1; i += 1.0 / n){
        p0 = transition(pos1, pos1 + dir1, i);
        p1 = transition(pos1 + dir1, pos2 + dir2, i);
        p2 = transition(pos2 + dir2, pos2, i);

        p0 = transition(p0, p1, i);
        p1 = transition(p1, p2, i);

        p0 = transition(p0, p1, i);
        drawLine(last_pos, p0);
        last_pos = p0;
    }
}
void drawRect(Rect rect){
    SDL_Rect _rect;
    _rect.x = rect.x;
    _rect.y = rect.y;
    _rect.w = rect.w;
    _rect.h = rect.h;
    SDL_RenderFillRect(renderer, &_rect);
}
void drawRect(SDL_Rect *rect){
    SDL_RenderFillRect(renderer, rect);
}
void drawRectBorder(SDL_Rect *rect, bool outside = false){
    if (outside){
        drawLine(rect->x - 1, rect->y - 1, rect->x + rect->w, rect->y - 1);
        drawLine(rect->x + rect->w, rect->y - 1, rect->x + rect->w, rect->y + rect->h);
        drawLine(rect->x - 1, rect->y - 1, rect->x - 1, rect->y + rect->h);
        drawLine(rect->x - 1, rect->y + rect->h, rect->x + rect->w, rect->y + rect->h);
    } else {
        drawLine(rect->x, rect->y, rect->x + rect->w - 1, rect->y);
        drawLine(rect->x + rect->w - 1, rect->y, rect->x + rect->w - 1, rect->y + rect->h - 1);
        drawLine(rect->x, rect->y, rect->x, rect->y + rect->h - 1);
        drawLine(rect->x, rect->y + rect->h - 1, rect->x + rect->w - 1, rect->y + rect->h - 1);
    }
}
void drawRectBorder(Rect rect, bool outside = false){
    if (outside){
        drawLine(rect.x - 1, rect.y - 1, rect.x + rect.w, rect.y - 1);
        drawLine(rect.x + rect.w, rect.y - 1, rect.x + rect.w, rect.y + rect.h);
        drawLine(rect.x - 1, rect.y - 1, rect.x - 1, rect.y + rect.h);
        drawLine(rect.x - 1, rect.y + rect.h, rect.x + rect.w, rect.y + rect.h);
    } else {
        drawLine(rect.x, rect.y, rect.x + rect.w - 1, rect.y);
        drawLine(rect.x + rect.w - 1, rect.y, rect.x + rect.w - 1, rect.y + rect.h - 1);
        drawLine(rect.x, rect.y, rect.x, rect.y + rect.h - 1);
        drawLine(rect.x, rect.y + rect.h - 1, rect.x + rect.w - 1, rect.y + rect.h - 1);
    }
}
void clearScreen(){
    SDL_RenderClear(renderer);
}
void updateScreen(){
    SDL_RenderPresent(renderer);
}
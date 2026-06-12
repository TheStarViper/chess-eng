import pygame
from variables import pieces


#https://www.dafont.com/chess.font

def draw_piece(piece, surface, fill_color, center_x, center_y,side, angle=0):
    symbol = ""
    
    match piece:
        case 1:  symbol = "o"  #Pawn
        case 2:  symbol = "j"  #Knight
        case 3:  symbol = "n"  #Bishop
        case 4:  symbol = "t"  #Rook
        case 5:  symbol = "w"  #Queen
        case 6:  symbol = "l"  #King
        case _:
            print(f"Unknown piece designation: {piece}")
            return

    if side == "WHITE":
        outline_surf = pieces.render(symbol, True, fill_color)
        if angle != 0:
            outline_surf = pygame.transform.rotate(outline_surf, angle)
            
        outline_rect = outline_surf.get_rect(center=(center_x, center_y))
        
        thickness = 2
        offsets = [
            (-thickness, 0), (thickness, 0), (0, -thickness), (0, thickness),
            (-thickness, -thickness), (-thickness, thickness), 
            (thickness, -thickness), (thickness, thickness)
        ]
        
        for dx, dy in offsets:
            surface.blit(outline_surf, (outline_rect.x + dx, outline_rect.y + dy))

    core_surf = pieces.render(symbol, True, fill_color)
    if angle != 0:
        core_surf = pygame.transform.rotate(core_surf, angle)
        
    core_rect = core_surf.get_rect(center=(center_x, center_y))
    surface.blit(core_surf, core_rect)